#ifndef QBloomRenderPass_h__
#define QBloomRenderPass_h__

#include "Render/IRenderPass.h"

class QBloomRenderPass :public IRenderPass {
	Q_OBJECT

	Q_BUILDER_BEGIN_RENDER_PASS(QBloomRenderPass, Raw, Blur)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QBloomRenderPass();
	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
};

#endif // QBloomRenderPass_h__
