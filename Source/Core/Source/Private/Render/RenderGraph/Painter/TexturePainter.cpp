//#include "TexturePainter.h"
//
//TexturePainter::TexturePainter()
//{
//}
//
//void TexturePainter::setupTexture(QRhiTexture* texture)
//{
//	mTexture = texture;
//}
//
//void TexturePainter::setupSampler(QRhiSampler::Filter magFilter /*= QRhiSampler::Nearest*/, QRhiSampler::Filter minFilter /*= QRhiSampler::Nearest*/, QRhiSampler::Filter mipmapMode /*= QRhiSampler::Nearest*/, QRhiSampler::AddressMode addressU /*= QRhiSampler::Repeat*/, QRhiSampler::AddressMode addressV /*= QRhiSampler::Repeat*/, QRhiSampler::AddressMode addressW /*= QRhiSampler::Repeat*/) {
//	mMagFilter = magFilter;
//	mMinFilter = minFilter;
//	mMipmapMode = mipmapMode;
//	mAddressU = addressU;
//	mAddressV = addressV;
//	mAddressW = addressW;
//}
//
//void TexturePainter::compile()
//{
//	mPipeline.reset(mRhi->newGraphicsPipeline());
//	QRhiGraphicsPipeline::TargetBlend blendState;
//	blendState.dstColor = QRhiGraphicsPipeline::One;
//	blendState.srcColor = QRhiGraphicsPipeline::One;
//	blendState.dstAlpha = QRhiGraphicsPipeline::One;
//	blendState.srcAlpha = QRhiGraphicsPipeline::One;
//	blendState.enable = true;
//	mPipeline->setTargetBlends({ blendState });
//	mPipeline->setSampleCount(mSampleCount);
//	mPipeline->setDepthTest(false);
//
//	QString vsCode = R"(#version 450
//		layout (location = 0) out vec2 vUV;
//		out gl_PerVertex{
//			vec4 gl_Position;
//		};
//		void main() {
//			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
//			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
//			%1
//		}
//	)";
//	QShader vs = QRhiHelper::newShaderFromCode( QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());
//
//	QShader fs = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
//		layout (binding = 0) uniform sampler2D uSamplerColor;
//		layout (location = 0) in vec2 vUV;
//		layout (location = 0) out vec4 outFragColor;
//		void main() {
//			outFragColor = vec4(texture(uSamplerColor, vUV).rgb,1.0f);
//		}
//	)");
//	mPipeline->setShaderStages({
//		{ QRhiShaderStage::Vertex, vs },
//		{ QRhiShaderStage::Fragment, fs }
//	});
//	QRhiVertexInputLayout inputLayout;
//
//	mSampler.reset(mRhi->newSampler(
//		mMagFilter,
//		mMinFilter,
//		mMipmapMode,
//		mAddressU,
//		mAddressV,
//		mAddressW));
//	mSampler->create();
//
//	mBindings.reset(mRhi->newShaderResourceBindings());
//	mBindings->setBindings({
//		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mTexture,mSampler.get())
//	});
//	mBindings->create();
//	mPipeline->setVertexInputLayout(inputLayout);
//	mPipeline->setShaderResourceBindings(mBindings.get());
//	mPipeline->setRenderPassDescriptor(mRenderPassDesc);
//	mPipeline->create();
//}
//
//void TexturePainter::paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) {
//	if (mTexture) {
//		cmdBuffer->setGraphicsPipeline(mPipeline.get());
//		cmdBuffer->setViewport(QRhiViewport(0, 0, renderTarget->pixelSize().width(), renderTarget->pixelSize().height()));
//		cmdBuffer->setShaderResources(mBindings.get());
//		cmdBuffer->draw(4);
//	}
//}
//
