#ifndef QToneMappingPassBuilder_h__
#define QToneMappingPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QToneMappingPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QToneMappingPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(float, Gamma) = 2.2f;
		QRP_INPUT_ATTR(float, Exposure) = 1.0f;
		QRP_INPUT_ATTR(float, PureWhite) = 1.0f;
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QToneMappingPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, ToneMappingReslut);
	QRP_OUTPUT_END()
public:
	QToneMappingPassBuilder();  
public:
	void setup(QRenderGraphBuilder& builder) override;

	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mToneMappingFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock{
		float gamma;
		float exposure;
		float pureWhite;
	};
	RTResource mRT;
	QRhiBufferRef mUniformBuffer;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QToneMappingPassBuilder_h__