#ifndef QBasePassDeferred_h__
#define QBasePassDeferred_h__


#include "Render/IRenderPass.h"

class QBasePassDeferred : public IBasePass {
	Q_OBJECT
public:
	enum OutSlot {
		BaseColor,
		Position,
		Normal,
		Tangent,
		Metalness,
		Roughness,
		Depth,
	#ifdef QENGINE_WITH_EDITOR
		DebugId,
	#endif
	};
	QBasePassDeferred();
protected:
	struct RT {
		QScopedPointer<QRhiTexture> atBaseColor;
		QScopedPointer<QRhiTexture> atPosition;
		QScopedPointer<QRhiTexture> atNormal;
		QScopedPointer<QRhiTexture> atTangent;
		QScopedPointer<QRhiTexture> atMetalness;
		QScopedPointer<QRhiTexture> atRoughness;
	#ifdef QENGINE_WITH_EDITOR
		QScopedPointer<QRhiTexture> atDebugId;
	#endif
		QScopedPointer<QRhiTexture> atDepthStencil;
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


#endif // QBasePassDeferred_h__
