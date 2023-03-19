#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "Render/RHI/QRhiEx.h"

class IBasePass;

class IRenderComponent: public QObject {
	Q_OBJECT
	friend class IBasePass;
public:
	QRhiEx::Signal sigonRebuildResource;
	QRhiEx::Signal sigonRebuildPipeline;
public:
	IRenderComponent():mID(IDStack++){ }
	uint32_t getID() const { return mID; }
	virtual bool isVaild() { return true; }
	virtual void onRebuildResource() {}
	virtual void onRebuildPipeline() {}
	virtual void onPreUpdate(QRhiCommandBuffer* cmdBuffer) {}
	virtual void onUpload(QRhiResourceUpdateBatch* batch) {}
	virtual void onUpdate(QRhiResourceUpdateBatch* batch) {}
	virtual void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;
	IBasePass* sceneRenderPass() { return mScreenRenderPass; }
protected:
	QRhiEx* mRhi;
	IBasePass* mScreenRenderPass = nullptr;
	uint32_t mID;
	inline static uint32_t IDStack = 0;
};


#endif // IRenderComponent_h__
