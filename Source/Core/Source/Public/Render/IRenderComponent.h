#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "Render/RHI/QRhiHelper.h"
#include "Utils/QObjectBuilder.h"
#include "QEngineCoreAPI.h"
#include "IRenderer.h"
#include "Render/QPrimitiveRenderProxy.h"


class QENGINECORE_API IRenderComponent: public QObject {
	Q_OBJECT
	friend class IBasePass;
public:
	QRhiSignal mSigRebuildResource;
public:
	IRenderComponent(){}

	void initialize(IRenderer* renderer, QRhiTextureRenderTarget* renderTarget);

	QSharedPointer<QPrimitiveRenderProxy> newPrimitiveRenderProxy();

	bool hasColorAttachment(const QString& name);
	int getColorAttachmentCount();
	int getSampleCount() const;
	QSize getPixelSize() const;
	QRhiRenderPassDescriptor* getRenderPassDesc() const;
	QRhi* getRhi() const;
	IRenderer* getRenderer() const;
	QRhiTextureRenderTarget* getRenderTarget() const;

	virtual void onPreRenderTick(QRhiCommandBuffer* cmdBuffer){}
	virtual void onRebuildResource() {}
protected:
	QRhi* mRhi = nullptr;
	IRenderer* mRenderer = nullptr;
	QRhiTextureRenderTarget* mRenderTarget = nullptr;
};

#endif // IRenderComponent_h__
