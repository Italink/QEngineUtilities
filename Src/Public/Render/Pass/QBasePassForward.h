#ifndef QBasePassForward_h__
#define QBasePassForward_h__

#include "Render/IRenderPass.h"

class QBasePassForward : public IBasePass {
	Q_OBJECT
public:
	enum  OutSlot {
		BaseColor,
#ifdef QENGINE_WITH_EDITOR
		DebugId,
#endif
	};
	QBasePassForward();
protected:
	struct RT {
		QScopedPointer<QRhiTexture> atBaseColor;
#ifdef QENGINE_WITH_EDITOR
		QScopedPointer<QRhiTexture> atDebugId;
#endif
		QScopedPointer<QRhiRenderBuffer> atDepthStencil;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	QList<QPair<QRhiTexture::Format, QString>> getRenderTargetSlots() override;
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
	QRhiRenderTarget* getRenderTarget() override;
	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
protected:
	RT mRT;
};

#endif // QBasePassForward_h__
