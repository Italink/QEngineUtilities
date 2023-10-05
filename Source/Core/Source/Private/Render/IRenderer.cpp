#include "IRenderer.h"
#include <QWindow>
#include "QRendererSurface.h"
#include "RHI/QRhiHelper.h"
#include <QMetaMethod>
#include "QRhiCamera.h"
#include "IRenderComponent.h"
#include "Render/RenderGraph/QRenderGraphBuilder.h"

class QRenderThreadWorkder : public QObject {
public:
	QRenderThreadWorkder(IRenderer* renderer) {
		mRenderer = renderer;
		mThread = new QThread();
		mThread->setObjectName("RenderThread");
		moveToThread(mThread);
		mThread->start();
	}
	void render(){
		QRhiCommandBuffer* cmdBuffer;
		QRhiRenderTarget* renderTarget;

		if (!mRenderer->mSurface->beginFrame(&cmdBuffer, &renderTarget))
			return;

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 1.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		mRenderer->mGraphBuilder->setMainRenderTarget(renderTarget);
		mRenderer->setupGraph(*mRenderer->mGraphBuilder.get());
		mRenderer->mGraphBuilder->compile();
		mRenderer->mGraphBuilder->execute(cmdBuffer);

		mRenderer->mSurface->endFrame();

		mRenderer->mGraphBuilder->clear();
		mCondition.wakeOne();
	}

	void initialize(){
		mRenderer->mRhi = QRhiHelper::create(mRenderer->mInitParams.backend, mRenderer->mInitParams.rhiFlags, mRenderer->maybeWindow());
		mRenderer->mCamera->setupRhi(mRenderer->mRhi.get());
		mRenderer->mSurface->initialize(mRenderer->mRhi.get(), mRenderer->mInitParams);
		mRenderer->mGraphBuilder = QSharedPointer<QRenderGraphBuilder>::create(mRenderer);
		mCondition.wakeOne();
	}

	void destroy(){
		mRenderer->mGraphBuilder.reset();
		mRenderer->mSurface->destroy();
		mCondition.wakeOne();
	}

public:
	IRenderer* mRenderer = nullptr;
	QThread* mThread = nullptr;
	QMutex mMutex;
	QWaitCondition mCondition;
};

IRenderer::IRenderer(QRhiHelper::InitParams params, QSize size, Type type /*= Type::Window*/)
	: mRenderThreadWorker(new QRenderThreadWorkder(this))
	, mInitParams(params)
{
	mCamera = new QRhiCamera();
	mCamera->setParent(this);
	if (type == Type::Window) {
		QRendererWindowSurface* windowSurface = new QRendererWindowSurface(params.backend, size);
		connect(windowSurface, &QRendererWindowSurface::renderRequested, mRenderThreadWorker.get(), &QRenderThreadWorkder::render);
		connect(windowSurface, &QRendererWindowSurface::initRequested, [this]() {
			mRenderThreadWorker->mMutex.lock();
			QMetaObject::invokeMethod(mRenderThreadWorker.get(), &QRenderThreadWorkder::initialize);
			mRenderThreadWorker->mCondition.wait(&mRenderThreadWorker->mMutex);
			mRenderThreadWorker->mMutex.unlock();
		});
		connect(windowSurface, &QRendererWindowSurface::surfaceGoingAway,  [this]() {
			mRenderThreadWorker->mMutex.lock();
			QMetaObject::invokeMethod(mRenderThreadWorker.get(), &QRenderThreadWorkder::destroy);
			mRenderThreadWorker->mCondition.wait(&mRenderThreadWorker->mMutex);
			mRenderThreadWorker->mMutex.unlock();
		});
		connect(windowSurface, &QRendererWindowSurface::syncSurfaceSizeRequested, [this]() {
			mRenderThreadWorker->mMutex.lock();
			QMetaObject::invokeMethod(mRenderThreadWorker.get(), &QRenderThreadWorkder::render);
			mRenderThreadWorker->mCondition.wait(&mRenderThreadWorker->mMutex);
			mRenderThreadWorker->mMutex.unlock();
		});
		mSurface = QSharedPointer<IRendererSurface>(windowSurface);
		mCamera->setupWindow(mSurface->maybeWindow());
	}
	else {
		QRendererOffscreenSurface* offscreenSurface = new QRendererOffscreenSurface(size);
		mSurface = QSharedPointer<IRendererSurface>(offscreenSurface, [this](IRendererSurface* surface) {
			mRenderThreadWorker->destroy();
			delete surface;
		});
		mRenderThreadWorker->initialize();
	}
}

QThread* IRenderer::renderThread()
{
	return mRenderThreadWorker->mThread;
}

QWindow* IRenderer::maybeWindow()
{
	return mSurface->maybeWindow();
}

QRhi* IRenderer::rhi()
{
	return mRhi.get();
}

QRhiCamera* IRenderer::getCamera()
{
	return mCamera;
}

void IRenderer::setCurrentObject(QObject* val)
{
	if (mCurrentObject != val) {
		mCurrentObject = val;
		Q_EMIT currentObjectChanged(val);
	}
}

void IRenderer::resize(const QSize& size)
{
	mSurface->resize(size);
}

const QVector<IRenderComponent*>& IRenderer::getRenderComponents()
{
	return mRenderComponents;
}

void IRenderer::addComponent(IRenderComponent* inRenderComponent)
{
	Q_ASSERT(inRenderComponent);
	inRenderComponent->setParent(this);
	mRenderComponents << inRenderComponent;
}

void IRenderer::removeComponent(IRenderComponent* inRenderComponent)
{
	mRenderComponents.removeAll(inRenderComponent);
}

const QVector<QPrimitiveRenderProxy*>& IRenderer::getRenderProxies()
{
	return mRenderProxies;
}

void IRenderer::registerPipeline(QPrimitiveRenderProxy* inProxy)
{
	mRenderProxies << inProxy;
}

void IRenderer::unregisterPipeline(QPrimitiveRenderProxy* inProxy)
{
	mRenderProxies.removeOne(inProxy);
}
