#ifndef QToneMappingPassBuilder_h__
#define QToneMappingPassBuilder_h__

#include "IRenderPassBuilder.h"

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
	void setup(QRenderGraphBuilder& builder) override
	{
		builder.setupTexture(mRT.colorAttachment, "ToneMappingTexture", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
		builder.setupRenderTarget(mRT.renderTarget, "ToneMappingRenderTarget", { mRT.colorAttachment.get() });

		builder.setupBuffer(mUniformBuffer, "GlslUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));

		builder.setupSampler(mSampler, "ToneMappingSampler",
			QRhiSampler::Linear,
			QRhiSampler::Linear,
			QRhiSampler::None,
			QRhiSampler::ClampToEdge,
			QRhiSampler::ClampToEdge);

		builder.setupShaderResourceBindings(mBindings, "ToneMappingBindings", {
			QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._BaseColorTexture.get(), mSampler.get()),
			QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
			});

		QRhiGraphicsPipelineState PSO;
		PSO.sampleCount = mRT.renderTarget->sampleCount();
		PSO.shaderResourceBindings = mBindings.get();
		PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
		PSO.shaderStages = {
			{ QRhiShaderStage::Vertex, builder.fullScreenVS() },
			{ QRhiShaderStage::Fragment, mToneMappingFS }
		};
		builder.setupGraphicsPipeline(mPipeline, "ToneMappingPipeline", PSO);

		mOutput.ToneMappingReslut = mRT.colorAttachment;
	}

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