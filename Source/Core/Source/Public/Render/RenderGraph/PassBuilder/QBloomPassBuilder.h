#ifndef QBloomPassBuilder_h__
#define QBloomPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QBloomPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QBloomPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(QRhiTextureRef, BlurTexture);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QBloomPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, BloomResult);
	QRP_OUTPUT_END()
public:
	QBloomPassBuilder();
protected:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mBloomFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	RTResource mRT;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QBloomPassBuilder_h__
