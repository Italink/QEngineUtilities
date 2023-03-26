#ifndef QBasePassDeferred_h__
#define QBasePassDeferred_h__

#include "Render/IRenderPass.h"

class QBasePassDeferred : public IBasePass {
	Q_OBJECT
public:

	Q_BUILDER_BEGIN_BASE_PASS(QBasePassDeferred)
#ifdef QENGINE_WITH_EDITOR
	Q_BUILDER_END_BASE_PASS(BaseColor, Position, Normal, Tangent, Metalness, Roughness, DebugId, Depth)
#else
	Q_BUILDER_END_BASE_PASS(BaseColor, Position, Normal, Tangent, Metalness, Roughness, Depth)
#endif
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
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
	QRhiRenderTarget* getRenderTarget() override;
	void resizeAndLinkNode(const QSize& size) override;
protected:
	RT mRT;
};


#endif // QBasePassDeferred_h__
