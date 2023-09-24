#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "Render/RHI/QRhiHelper.h"
#include "Utils/QObjectBuilder.h"
#include "QEngineCoreAPI.h"
#include "IRenderer.h"

class QENGINECORE_API IRenderComponent: public QObject {
	Q_OBJECT
	friend class IBasePass;
public:
	QRhiSignal mSigRebuildResource;
	QRhiSignal mSigRebuildPipeline;
public:
	IRenderComponent(): mID(++IDStack) { }

	void initialize(IRenderer* renderer, QRhiTextureRenderTarget* renderTarget) {
		mRhi = renderer->rhi();
		mRenderer = renderer;
		mRenderTarget = renderTarget;
	}

	bool hasColorAttachment(const QString& name) {
		for (int i = 0; i < mRenderTarget->description().colorAttachmentCount(); i++) {
			auto colorAttach = mRenderTarget->description().colorAttachmentAt(i);
			if (colorAttach->texture()->name() == name)
				return true;
		}
		return false;
	}

	int getColorAttachmentCount() { return mRenderTarget->description().colorAttachmentCount(); }
	int getSampleCount() const { return mRenderTarget->sampleCount(); }
	QSize getPixelSize() const { return mRenderTarget->pixelSize(); }
	QRhiRenderPassDescriptor* getRenderPassDesc() const { return mRenderTarget->renderPassDescriptor(); }
	QRhi* getRhi() const { return mRhi; }
	QRhiTextureRenderTarget* getRenderTarget() const { return mRenderTarget; }

	uint32_t getID() const { return mID; }
	virtual void onRebuildResource() {}
	virtual void onRebuildPipeline() {}
	virtual void onPreUpdate(QRhiCommandBuffer* cmdBuffer) {}
	virtual void onUpload(QRhiResourceUpdateBatch* batch) {}
	virtual void onUpdate(QRhiResourceUpdateBatch* batch) {}
	virtual void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;
protected:
	QRhi* mRhi = nullptr;
	IRenderer* mRenderer = nullptr;
	QRhiTextureRenderTarget* mRenderTarget = nullptr;
	uint32_t mID = 0;
	inline static uint32_t IDStack = 0;
};


#endif // IRenderComponent_h__
