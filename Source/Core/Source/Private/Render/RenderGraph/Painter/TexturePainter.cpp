#include "TexturePainter.h"

TexturePainter::TexturePainter()
{
	mTextureFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
			layout (binding = 0) uniform sampler2D uSamplerColor;
			layout (location = 0) in vec2 vUV;
			layout (location = 0) out vec4 outFragColor;
			void main() {
				outFragColor = vec4(texture(uSamplerColor, vUV).rgb,1.0f);
			}
	)");
}

void TexturePainter::setupTexture(QRhiTexture* texture)
{
	mTexture = texture;
}

void TexturePainter::setupSampler(QRhiSampler::Filter magFilter /*= QRhiSampler::Nearest*/, QRhiSampler::Filter minFilter /*= QRhiSampler::Nearest*/, QRhiSampler::Filter mipmapMode /*= QRhiSampler::Nearest*/, QRhiSampler::AddressMode addressU /*= QRhiSampler::Repeat*/, QRhiSampler::AddressMode addressV /*= QRhiSampler::Repeat*/, QRhiSampler::AddressMode addressW /*= QRhiSampler::Repeat*/) {
	mMagFilter = magFilter;
	mMinFilter = minFilter;
	mMipmapMode = mipmapMode;
	mAddressU = addressU;
	mAddressV = addressV;
	mAddressW = addressW;
}

void TexturePainter::setup(QRenderGraphBuilder& builder, QRhiRenderTarget* rt)
{
	builder.setupSampler(mSampler, "TextureSampler",
		mMagFilter,
		mMinFilter,
		mMipmapMode,
		mAddressU,
		mAddressV,
		mAddressW);

	builder.setupShaderResourceBindings(mBindings, "TextureBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mTexture,mSampler.get())
	});

	QRhiGraphicsPipelineState PSO;

	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	blendState.dstColor = QRhiGraphicsPipeline::One;
	blendState.srcColor = QRhiGraphicsPipeline::One;
	blendState.dstAlpha = QRhiGraphicsPipeline::One;
	blendState.srcAlpha = QRhiGraphicsPipeline::One;
	PSO.targetBlends = { blendState };

	PSO.sampleCount = rt->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = rt->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mTextureFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "TexturePainterPipeline", PSO);
}

void TexturePainter::resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi)
{

}

void TexturePainter::paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget)
{
	if (mTexture) {
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, renderTarget->pixelSize().width(), renderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mBindings.get());
		cmdBuffer->draw(4);
	}
}
