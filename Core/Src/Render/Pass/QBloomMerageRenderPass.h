#ifndef QBloomMerageRenderPass_h__
#define QBloomMerageRenderPass_h__

#include "Render/IRenderPass.h"

class QBloomMerageRenderPass :public IRenderPass {
	Q_OBJECT

	Q_BUILDER_BEGIN_RENDER_PASS(QBloomMerageRenderPass, Raw, Blur)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QBloomMerageRenderPass();

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

#endif // QBloomMerageRenderPass_h__
