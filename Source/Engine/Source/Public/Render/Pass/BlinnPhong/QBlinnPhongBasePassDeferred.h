#ifndef QBlinnPhongBasePassDeferred_h__
#define QBlinnPhongBasePassDeferred_h__

#include "Render/IRenderPass.h"
#include "QEngineUtilitiesAPI.h"

class QENGINEUTILITIES_API QBlinnPhongBasePassDeferred : public IBasePass {
	Q_OBJECT
public:

	Q_BUILDER_BEGIN_BASE_PASS(QBlinnPhongBasePassDeferred)
#ifdef QENGINE_WITH_EDITOR
	Q_BUILDER_END_BASE_PASS(BaseColor, Position, Normal, Specular, DebugId, Depth)
#else
	Q_BUILDER_END_BASE_PASS(BaseColor, Position, Normal, Specular, Depth)
#endif
	QBlinnPhongBasePassDeferred();
	~QBlinnPhongBasePassDeferred();
protected:
	struct RT {
		QScopedPointer<QRhiTexture> atBaseColor;
		QScopedPointer<QRhiTexture> atPosition;
		QScopedPointer<QRhiTexture> atNormal;
		QScopedPointer<QRhiTexture> atSpecular;
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


#endif // QBlinnPhongBasePassDeferred_h__
