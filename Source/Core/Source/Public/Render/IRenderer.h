#ifndef QRenderer_h__
#define QRenderer_h__

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "Render/RenderGraph/QRGBuilder.h"
#include "QCamera.h"
#include "Render/RHI/QRhiHelper.h"

class IRendererSurface;
class QRenderThreadWorkder;

class QENGINECORE_API IRenderer : public QObject {
public:
	friend class QRenderThreadWorkder;
	enum class Type {
		Window,
		Offscreen
	};

	IRenderer(QRhiHelper::InitParams params, QSize size, Type type = Type::Window);

	QWindow* maybeWindow();
	QRhi* rhi();
	QCamera* getCamera();
	void resize(const QSize& size);
protected:
	virtual void setupGraph(QRGBuilder& graphBuilder) {}
private:
	QRhiHelper::InitParams mInitParams;
	QCamera* mCamera = nullptr;
	QSharedPointer<QRhi> mRhi;
	QSharedPointer<QRGBuilder> mGraphBuilder;
	QSharedPointer<IRendererSurface> mSurface;
	QSharedPointer<QRenderThreadWorkder> mRenderThreadWorker;
};


#endif // QRenderer_h__
