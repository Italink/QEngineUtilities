#include "QRendererSurface.h"
#include <QPlatformSurfaceEvent>
#include <QGuiApplication>

QRendererWindowSurface::QRendererWindowSurface(QRhi::Implementation impl, QSize size)
{
	resize(size);
	switch (impl) {
	case QRhi::OpenGLES2:
		setSurfaceType(OpenGLSurface);
		break;
	case QRhi::Vulkan:
		setSurfaceType(VulkanSurface);
		break;
	case QRhi::D3D11:
	case QRhi::D3D12:
		setSurfaceType(Direct3DSurface);
		break;
	case QRhi::Metal:
		setSurfaceType(MetalSurface);
		break;
	default:
		break;
	}
}

void QRendererWindowSurface::exposeEvent(QExposeEvent*)
{
	if (isExposed()) {
		if (!mRunning) {
			// initialize and start rendering when the window becomes usable for graphics purposes
			mRunning = true;
			mNotExposed = false;
			mNewlyExposed = true;
			emit initRequested();
			emit renderRequested();
		}
		else {
			// continue when exposed again
			if (mNotExposed) {
				mNotExposed = false;
				mNewlyExposed = true;
				emit renderRequested();
			}
			else {
				// resize generates exposes - this is very important here (unlike in a single-threaded renderer)
				emit syncSurfaceSizeRequested();
			}
		}
	}
	else {
		// stop pushing frames when not exposed (on some platforms this is essential, optional on others)
		if (mRunning)
			mNotExposed = true;
	}
}

bool QRendererWindowSurface::event(QEvent* e)
{
	switch (e->type()) {
	case QEvent::UpdateRequest:
		if (!mNotExposed) {
			mNewlyExposed = false;
			emit renderRequested();
		}
		break;

	case QEvent::PlatformSurface:
		// this is the proper time to tear down the swapchain (while the native window and surface are still around)
		if (static_cast<QPlatformSurfaceEvent*>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
			emit surfaceGoingAway();
		break;

	default:
		break;
	}

	return QWindow::event(e);
}

QWindow* QRendererWindowSurface::maybeWindow()
{
	return this;
}


void QRendererWindowSurface::resize(const QSize& size)
{
	QWindow::resize(size);
}

void QRendererWindowSurface::initialize(QRhi* rhi, QRhiHelper::InitParams initParams)
{
	mRhi = rhi;
	mBeginFrameFlags = initParams.beginFrameFlags;
	mEndFrameFlags = initParams.endFrameFlags;

	mSwapChain.reset(mRhi->newSwapChain());
	mDSBuffer.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil, QSize(), initParams.sampleCount, QRhiRenderBuffer::UsedWithSwapChainOnly));
	mSwapChain->setWindow(this);
	mSwapChain->setDepthStencil(mDSBuffer.get());
	mSwapChain->setSampleCount(initParams.sampleCount);
	mSwapChain->setFlags(initParams.swapChainFlags);
	mSwapChainPassDesc.reset(mSwapChain->newCompatibleRenderPassDescriptor());
	mSwapChain->setRenderPassDescriptor(mSwapChainPassDesc.get());
	mHasSwapChain = mSwapChain->createOrResize();
}

void QRendererWindowSurface::destroy()
{
	mDSBuffer.reset();
	mSwapChainPassDesc.reset();
	mSwapChain.reset();
}

bool QRendererWindowSurface::beginFrame(QRhiCommandBuffer** outCmdBuffer, QRhiRenderTarget** outRenderTarget)
{
	if (!mHasSwapChain || mNotExposed)
		return false;
	// If the window got resized or got newly exposed, resize the swapchain.
	// (the newly-exposed case is not actually required by some
	// platforms/backends, but f.ex. Vulkan on Windows seems to need it)
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize()) {
		mHasSwapChain = mSwapChain->createOrResize() ;
		if (!mHasSwapChain)
			return false;
		mNewlyExposed = false;
	}

	static int CpuFrameCounter = 0;
	QRhi::FrameOpResult r = mRhi->beginFrame(mSwapChain.get(), mBeginFrameFlags);
	if (r == QRhi::FrameOpSwapChainOutOfDate) {
		mHasSwapChain = mSwapChain->createOrResize();
		if (!mHasSwapChain)
			return false;
		r = mRhi->beginFrame(mSwapChain.get());
	}
	if (r != QRhi::FrameOpSuccess) {
		requestUpdate();
		return false;
	}

	*outCmdBuffer = mSwapChain->currentFrameCommandBuffer();
	*outRenderTarget = mSwapChain->currentFrameRenderTarget();
	return true;
}

void QRendererWindowSurface::endFrame()
{
	mRhi->endFrame(mSwapChain.get(), mEndFrameFlags);
	QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

QRendererOffscreenSurface::QRendererOffscreenSurface(QSize size)
{
	mRequestSize = size;
	resize(size);
}

void QRendererOffscreenSurface::resize(const QSize& size)
{
	if (mRhi) {
		mColorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, mRequestSize, mRequestSampleCount, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mColorAttachment->create();
		mDepthStencilAttachment.reset(mRhi->newTexture(QRhiTexture::D24S8, mRequestSize, mRequestSampleCount, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mDepthStencilAttachment->create();

		QRhiTextureRenderTargetDescription RTDesc;
		RTDesc.setColorAttachments({ QRhiColorAttachment(mColorAttachment.get()) });

		RTDesc.setDepthTexture(mDepthStencilAttachment.get());
		mRenderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
		mRenderPassDesc.reset(mRenderTarget->newCompatibleRenderPassDescriptor());
		mRenderTarget->setRenderPassDescriptor(mRenderPassDesc.get());
		mRenderTarget->create();
	}
}

void QRendererOffscreenSurface::initialize(QRhi* rhi, QRhiHelper::InitParams initParams)
{
	mRhi = rhi;
	mRequestSampleCount = initParams.sampleCount;
	mBeginFrameFlags = initParams.beginFrameFlags;
	mEndFrameFlags = initParams.endFrameFlags;
	resize(mRequestSize);
}

void QRendererOffscreenSurface::destroy()
{
	mColorAttachment.reset();
	mDepthStencilAttachment.reset();
	mRenderTarget.reset();
	mRenderPassDesc.reset();
}

bool QRendererOffscreenSurface::beginFrame(QRhiCommandBuffer** outCmdBuffer, QRhiRenderTarget** outRenderTarget)
{
	*outRenderTarget = mRenderTarget.get();
	return mRhi->beginOffscreenFrame(outCmdBuffer, mBeginFrameFlags) == QRhi::FrameOpSuccess;
}

void QRendererOffscreenSurface::endFrame()
{
	mRhi->endOffscreenFrame(mEndFrameFlags);
}
