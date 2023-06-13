#ifndef QPbrBasePassDeferred_h__
#define QPbrBasePassDeferred_h__

#include "Render/IRenderPass.h"
#include "QEngineUtilitiesAPI.h"

class QENGINEUTILITIES_API QPbrBasePassDeferred : public IBasePass {
	Q_OBJECT
public:

	Q_BUILDER_BEGIN_BASE_PASS(QPbrBasePassDeferred)
#ifdef QENGINE_WITH_EDITOR
	Q_BUILDER_END_BASE_PASS(BaseColor, Position, Normal, Metallic, Roughness, DebugId, Depth)
#else
	Q_BUILDER_END_BASE_PASS(BaseColor, Position, Normal, Metallic, Roughness, Depth)
#endif
	QPbrBasePassDeferred();
protected:
	struct RT {
		QScopedPointer<QRhiTexture> atBaseColor;
		QScopedPointer<QRhiTexture> atPosition;
		QScopedPointer<QRhiTexture> atNormal;
		QScopedPointer<QRhiTexture> atMetallic;
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


#endif // QPbrBasePassDeferred_h__
