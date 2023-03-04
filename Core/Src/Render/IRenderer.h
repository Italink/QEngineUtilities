#ifndef IRenderer_h__
#define IRenderer_h__

#include "QFrameGraph.h"
#include "Render/RHI/QRhiEx.h"
#include "Utils/QCamera.h"

class TexturePainter;
class IRenderComponent;

class IRenderer: public QObject {
	Q_OBJECT
	Q_PROPERTY(QCamera* Camera READ getCamera WRITE setCamera)
public:
	IRenderer(QRhiEx* inRhi,const QSize& inFrameSize);
	~IRenderer(){}
	void requestComplie();

	virtual QRhiRenderTarget* renderTaget() = 0;
	virtual QRhiRenderPassDescriptor* renderPassDescriptor() { return renderTaget()->renderPassDescriptor(); }
	virtual int sampleCount() = 0;

	void setCamera(QCamera* inCamera);
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);
	void setCurrentObject(QObject* val);

	QSize getFrameSize() const { return mFrameSize; }
	QCamera* getCamera() const { return mCamera; }
	IRenderPassBase* getRenderPassByName(const QString& inName);
	QRhiEx* getRhi() { return mRhi; }
	QObject* getCurrentObject() const { return mCurrentObject; }
	IRenderComponent* getComponentById(uint32_t inId);
	QFrameGraph* getFrameGarph() const { return mFrameGraph.get(); }
Q_SIGNALS:
	void asCurrentObjectChanged(QObject*);
protected:
	virtual QRhiCommandBuffer* commandBuffer() = 0;
	virtual void compile();
	virtual void render();
	virtual void resize(const QSize& size);
	virtual void refreshOutputTexture();
protected:
	QSize mFrameSize;
	QRhiEx* mRhi;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QSharedPointer<TexturePainter> mOutputPainter;
	QCamera* mCamera = nullptr;
	bool bRequestCompile = false;
	QObject* mCurrentObject = nullptr;
};

#endif // IRenderer_h__
