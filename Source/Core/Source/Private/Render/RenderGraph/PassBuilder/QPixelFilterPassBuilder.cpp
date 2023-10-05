#include "QPixelFilterPassBuilder.h"

QPixelFilterPassBuilder::QPixelFilterPassBuilder()
{
}

void QPixelFilterPassBuilder::setup(QRenderGraphBuilder& builder)
{
	if (mFilterCode != mInput._FilterCode) {
		mFilterCode = mInput._FilterCode;
		mFilterFS = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
			layout (binding = 0) uniform sampler2D uTexture;
			layout (location = 0) in vec2 vUV;
			layout (location = 0) out vec4 outFragColor; )" 
			+ mFilterCode.toLocal8Bit());
	}
	if (!mFilterFS.isValid())
		return;

	builder.setupTexture(mRT.colorAttachment, "FilterTexture", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "FilterRenderTarget", { mRT.colorAttachment.get() });
	builder.setupSampler(mSampler, "FilterSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindings, "FilterBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._BaseColorTexture.get(), mSampler.get()),
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mFilterFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "FilterPipeline", PSO);
	mOutput.FilterResult = mRT.colorAttachment;
}

void QPixelFilterPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	if (!mFilterFS.isValid())
		return;
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
