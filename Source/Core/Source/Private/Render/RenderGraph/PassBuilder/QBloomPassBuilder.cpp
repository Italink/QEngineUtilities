#include "QBloomPassBuilder.h"

QBloomPassBuilder::QBloomPassBuilder()
{
	mBloomFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (binding = 0) uniform sampler2D uBaseColorTexture;
		layout (binding = 1) uniform sampler2D uBloomTexture;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		void main() {
			vec4 srcColor = texture(uBaseColorTexture, vUV);
			vec4 bloomColor = texture(uBloomTexture, vUV);
			outFragColor = vec4(srcColor.rgb+bloomColor.rgb,1.0);
		}
	)");
}

void QBloomPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mRT.colorAttachment, "BloomTexture", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize() , 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "BloomRenderTarget", { mRT.colorAttachment.get() });
	
	builder.setupSampler(mSampler, "BloomSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindings, "BloomBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._BaseColorTexture.get(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,mInput._BlurTexture.get(),mSampler.get())
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mBloomFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "BloomPipeline", PSO);
	mOutput.BloomResult = mRT.colorAttachment;
}

void QBloomPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
