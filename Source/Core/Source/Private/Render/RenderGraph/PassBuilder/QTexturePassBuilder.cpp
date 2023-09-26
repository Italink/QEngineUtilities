#include "QTexturePassBuilder.h"

QTexutrePassBuilder::QTexutrePassBuilder()
{
	mFragmentShader = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
		layout (binding = 0) uniform sampler2D uSamplerColor;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		void main() {
			outFragColor = vec4(texture(uSamplerColor, vUV).rgb,1.0f);
		}
	)");
	Q_ASSERT(mFragmentShader.isValid());
}

void QTexutrePassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupSampler(mSampler,"CopyPassSamper", QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None, QRhiSampler::Repeat, QRhiSampler::Repeat, QRhiSampler::Repeat);
	builder.setupShaderResourceBindings(mShaderBindings, "CopyPassBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._BaseColorTexture.get() ,mSampler.get())
	});
	QRhiGraphicsPipelineState PSO;
	PSO.shaderResourceBindings = mShaderBindings.get();													
	PSO.sampleCount = mInput._DstRenderTarget->sampleCount();
	PSO.renderPassDesc = mInput._DstRenderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		QRhiShaderStage(QRhiShaderStage::Vertex, builder.fullScreenVS()),
		QRhiShaderStage(QRhiShaderStage::Fragment, mFragmentShader)
	};
	builder.setupGraphicsPipeline(mPipeline, "CopyPassPipeline", PSO);
}

void QTexutrePassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	if (mInput._BaseColorTexture) {
		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cmdBuffer->beginPass(mInput._DstRenderTarget, clearColor, dsClearValue, nullptr);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mInput._DstRenderTarget->pixelSize().width(), mInput._DstRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mShaderBindings.get());
		cmdBuffer->draw(4);

		cmdBuffer->endPass();
	}													
}
