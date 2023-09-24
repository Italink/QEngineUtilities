//#ifndef QPixelFilterRenderPass_h__
//#define QPixelFilterRenderPass_h__
//
//#include "Render/IRenderPass.h"
//
//class QENGINECORE_API QPixelFilterRenderPass : public IRenderPass {
//	Q_OBJECT
//		Q_PROPERTY(QString FilterCode READ getFilterCode WRITE setFilterCode)
//
//	Q_BUILDER_BEGIN_RENDER_PASS(QPixelFilterRenderPass, Src)
//		Q_BUILDER_ATTRIBUTE(QString, FilterCode)
//	Q_BUILDER_END_RENDER_PASS(Result)
//public:
//	void setFilterCode(QString code);
//	QString getFilterCode();
//protected:
//	void resizeAndLinkNode(const QSize& size) override;
//	void compile() override;
//	void render(QRhiCommandBuffer* cmdBuffer) override;
//
//	struct RTResource {
//		QScopedPointer<QRhiTexture> colorAttachment;
//		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
//		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
//	};
//	RTResource mRT;
//	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
//	QScopedPointer<QRhiSampler> mSampler;
//	QScopedPointer<QRhiShaderResourceBindings> mBindings;
//	QString mFilterCode;
//	QRhiSignal sigRecompile;
//};
//
//#endif // QPixelFilterRenderPass_h__