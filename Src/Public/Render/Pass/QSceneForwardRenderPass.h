#ifndef QSceneForwardRenderPass_h__
#define QSceneForwardRenderPass_h__

#include "Render/IRenderPass.h"

class QSceneForwardRenderPass : public ISceneRenderPass {
	Q_OBJECT
public:
	enum  OutSlot {
		BaseColor,
#ifdef QENGINE_WITH_EDITOR
		DebugId,
#endif
	};
	QSceneForwardRenderPass();
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

#endif // QSceneForwardRenderPass_h__
