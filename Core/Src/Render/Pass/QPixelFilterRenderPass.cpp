#include "Render/Pass/QPixelFilterRenderPass.h"

void QPixelFilterRenderPass::setFilterCode(QString code) {
	mFilterCode = code;
	sigRecompile.request();
}

QString QPixelFilterRenderPass::getFilterCode() {
	return mFilterCode;
}

void QPixelFilterRenderPass::resizeAndLinkNode(const QSize& size) {
	QRhiTexture* src = getTextureIn_Src();
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, src->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_Result(mRT.colorAttachment.get());

	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));

	mSampler->create();

	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, src, mSampler.get())
	});
	mBindings->create();

	sigRecompile.receive();
}

void QPixelFilterRenderPass::compile() {
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	mPipeline->setTargetBlends({ blendState });
	mPipeline->setSampleCount(mRT.renderTarget->sampleCount());

	QString vsCode = R"(#version 450
		layout (location = 0) out vec2 vUV;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
			%1
		}
	)";

	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());
	QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (binding = 0) uniform sampler2D uTexture;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
	)" + mFilterCode.toLocal8Bit());

	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	QRhiVertexInputLayout inputLayout;
	mPipeline->setVertexInputLayout(inputLayout);
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
	mPipeline->create();
}

void QPixelFilterRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	if (sigRecompile.receive()) {
		compile();
	}
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
