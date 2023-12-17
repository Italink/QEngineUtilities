#ifndef QRenderer_h__
#define QRenderer_h__

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "Render/RHI/QRhiHelper.h"
#include "Utils/QRhiCamera.h"

class IRendererSurface;
class QRenderThreadWorkder;
class IRenderComponent;
class QPrimitiveRenderProxy;
class QRenderGraphBuilder;

class QENGINECORE_API IRenderer : public QObject {
	Q_OBJECT
public:
	friend class QRenderThreadWorkder;
	enum class Type {
		Window,
		Offscreen
	};

	IRenderer(QRhiHelper::InitParams params, QSize size = QSize(800, 600), Type type = Type::Window);

	QThread* renderThread();
	QWindow* maybeWindow();
	QRhi* rhi();
	QRhiCamera* getCamera();
	QRenderGraphBuilder* getRenderGraphBuilder() const { return mGraphBuilder.get(); }
	QObject* getCurrentObject() const { return mCurrentObject; }
	float getDeltaSec() const;
	void resetTimer();
	void setCurrentObject(QObject* val);
	void resize(const QSize& size);

	const QVector<IRenderComponent*>& getRenderComponents();
	void addComponent(IRenderComponent* inRenderComponent);
	void removeComponent(IRenderComponent* inRenderComponent);

	const QVector<QPrimitiveRenderProxy*>& getRenderProxies();
	void registerPipeline(QPrimitiveRenderProxy* inProxy);
	void unregisterPipeline(QPrimitiveRenderProxy* inProxy);
Q_SIGNALS:
	void currentObjectChanged(QObject*);
protected:
	virtual void setupGraph(QRenderGraphBuilder& graphBuilder) {}
	virtual void endFrame(){}
private:
	QRhiHelper::InitParams mInitParams;
	QRhiCamera* mCamera = nullptr;
	QObject* mCurrentObject = nullptr;
	QSharedPointer<QRhi> mRhi;
	QSharedPointer<QRenderGraphBuilder> mGraphBuilder;
	QSharedPointer<IRendererSurface> mSurface;
	QSharedPointer<QRenderThreadWorkder> mRenderThreadWorker;
	QVector<IRenderComponent*> mRenderComponents;
	QVector<QPrimitiveRenderProxy*> mRenderProxies;
	QElapsedTimer mTimer;
	int64_t mLastTimeMsec = 0.0;
	float mDeltaSec = 0.0f;
};


#endif // QRenderer_h__
