#include "QGuiApplication"
#include "QRhiWindow.h"
#include <QPlatformSurfaceEvent>
#include "Vulkan\QRhiVulkanExHelper.h"

QRhiWindow::QRhiWindow(QRhiWindow::InitParams inInitParmas)
	: mInitParams(inInitParmas)
{
	switch (mInitParams.backend) {
	case QRhi::OpenGLES2:
		setSurfaceType(OpenGLSurface);
		break;
	case QRhi::Vulkan: 
		setSurfaceType(VulkanSurface);
		break;
	case QRhi::D3D11:
		setSurfaceType(Direct3DSurface);
		break;
	case QRhi::Metal:
		setSurfaceType(MetalSurface);
		break;
	default:
		break;
	}
}

QRhiWindow::~QRhiWindow() {
	mDSBuffer.reset();
	mSwapChainPassDesc.reset();
	mRhi.reset();
}

void QRhiWindow::initializeInternal() {
	mRhi = QRhiEx::newRhiEx(mInitParams.backend, mInitParams.rhiFlags, this);
	if (!mRhi)
		qFatal("Failed to create RHI backend");

	mSwapChain.reset(mRhi->newSwapChain());
	mDSBuffer.reset( mRhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
		QSize(), 
		mInitParams.sampleCount,
		QRhiRenderBuffer::UsedWithSwapChainOnly));

	mSwapChain->setWindow(this);
	mSwapChain->setDepthStencil(mDSBuffer.get());
	mSwapChain->setSampleCount(mInitParams.sampleCount);
	mSwapChain->setFlags(mInitParams.swapChainFlags);
	mSwapChainPassDesc.reset( mSwapChain->newCompatibleRenderPassDescriptor());
	mSwapChain->setRenderPassDescriptor(mSwapChainPassDesc.get());
	mSwapChain->createOrResize();
	onInit();
	mHasSwapChain = true;
	if (mInitParams.enableStat) {
		mCpuFrameTimer.start();
		mRhi->addGpuFrameTimeCallback([this](float elapsedMs) {
			mGpuFrameTime = elapsedMs;
		});
	}
}

void QRhiWindow::renderInternal() {
	if (!mHasSwapChain || mNotExposed)
		return;

	// If the window got resized or got newly exposed, resize the swapchain.
	// (the newly-exposed case is not actually required by some
	// platforms/backends, but f.ex. Vulkan on Windows seems to need it)
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize() || mNewlyExposed) {
		resizeInternal();
		if (!mHasSwapChain)
			return;
		mNewlyExposed = false;
	}

	if (mNeedResize && !(QGuiApplication::mouseButtons() & Qt::LeftButton) ) {
		onResize(mSwapChain->currentPixelSize());
	}

	static int CpuFrameCounter = 0;
	QRhi::FrameOpResult r = mRhi->beginFrame(mSwapChain.get(), mInitParams.beginFrameFlags);
	if (r == QRhi::FrameOpSwapChainOutOfDate) {
		resizeInternal();
		if (!mHasSwapChain)
			return;
		if (mInitParams.enableStat) {
			CpuFrameCounter = 0;
			mCpuFrameTimer.restart();
		}
		r = mRhi->beginFrame(mSwapChain.get());
	}
	if (r != QRhi::FrameOpSuccess) {
		requestUpdate();
		return;
	}

	if (mInitParams.enableStat) {
		static float TimeCounter = 0;
		CpuFrameCounter += 1;
		mCpuFrameTime = mCpuFrameTimer.elapsed();
		TimeCounter += mCpuFrameTime;
		mCpuFrameTimer.restart();

		if (TimeCounter > 1000) {
			mFps = CpuFrameCounter;
			CpuFrameCounter = 0;
			TimeCounter = 0;
		}
	}

	onRenderTick();

	mRhi->endFrame(mSwapChain.get(), mInitParams.endFrameFlags);

	if (!mInitParams.swapChainFlags.testFlag(QRhiSwapChain::NoVSync))
		requestUpdate();
	else // try prevent all delays when NoVSync
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

void QRhiWindow::resizeInternal() {
	mHasSwapChain = mSwapChain->createOrResize();
	mNeedResize = true;
}

void QRhiWindow::exposeEvent(QExposeEvent*)
{
	// initialize and start rendering when the window becomes usable for graphics purposes
	if (isExposed()) {
		if (!mRunning) {
			mRunning = true;
			initializeInternal();
		}
		mNotExposed = false;
	}
	const QSize surfaceSize = mHasSwapChain ? mSwapChain->surfacePixelSize() : QSize();

	// stop pushing frames when not exposed (or size is 0)
	if ((!isExposed() || (mHasSwapChain && surfaceSize.isEmpty())) && mRunning)
		mNotExposed = true;

	// always render a frame on exposeEvent() (when exposed) in order to update
	// immediately on window resize.
	if (isExposed() && !surfaceSize.isEmpty())
		renderInternal();
}

bool QRhiWindow::event(QEvent* e)
{
	switch (e->type()) {
	case QEvent::UpdateRequest:
		if (mHasSwapChain) {
			renderInternal();
		}
		break;
	case QEvent::PlatformSurface:
		if (static_cast<QPlatformSurfaceEvent*>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
			mHasSwapChain = false;
			onExit();
			mSwapChain.reset();
		}
		break;
	default:
		break;
	}
	return QWindow::event(e);
}