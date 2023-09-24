//#ifndef QBasePassForward_h__
//#define QBasePassForward_h__
//
//#include "Render/IRenderPass.h"
//#include "QEngineUtilitiesAPI.h"
//
//class QENGINEUTILITIES_API QBasePassForward : public IBasePass {
//	Q_OBJECT
//public:
//
//	Q_BUILDER_BEGIN_BASE_PASS(QBasePassForward)
//#ifdef QENGINE_WITH_EDITOR
//		Q_BUILDER_END_BASE_PASS(BaseColor, DebugId)
//#else
//		Q_BUILDER_END_BASE_PASS(BaseColor)
//#endif
//
//	QBasePassForward();
//protected:
//	struct RT {
//		QScopedPointer<QRhiTexture> atBaseColor;
//#ifdef QENGINE_WITH_EDITOR
//		QScopedPointer<QRhiTexture> atDebugId;
//#endif
//		QScopedPointer<QRhiRenderBuffer> atDepthStencil;
//		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
//		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
//	};
//
//	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
//	QRhiRenderTarget* getRenderTarget() override;
//	void resizeAndLinkNode(const QSize& size) override;
//protected:
//	RT mRT;
//};
//
//#endif // QBasePassForward_h__
