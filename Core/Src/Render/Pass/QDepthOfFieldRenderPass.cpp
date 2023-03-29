#include "Render/Pass/QDepthOfFieldRenderPass.h"

QDepthOfFieldRenderPass::QDepthOfFieldRenderPass() {
}

void QDepthOfFieldRenderPass::setScreenFocusPoint(QVector2D val) {
	mParams.screenFocusPoint = val;
	sigUpdateParams.request();
}

void QDepthOfFieldRenderPass::setNear(float val) {
	mParams.near = val;
	sigUpdateParams.request();
}

void QDepthOfFieldRenderPass::setFar(float val) {
	mParams.far = val;
	sigUpdateParams.request();
}

void QDepthOfFieldRenderPass::resizeAndLinkNode(const QSize& size) {
	QRhiTexture* focus = getTextureIn_Focus();
	QRhiTexture* loseFocus = getTextureIn_LoseFocus();
	QRhiTexture* position = getTextureIn_Position();
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, focus->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
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

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
	mUniformBuffer->create();

	mSampler->create();

	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, focus, mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, loseFocus, mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(2,QRhiShaderResourceBinding::FragmentStage, position, mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer (3,QRhiShaderResourceBinding::FragmentStage, mUniformBuffer.get()),
	});
	mBindings->create();
	sigUpdateParams.request();
}

void QDepthOfFieldRenderPass::compile() {
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
		layout (binding = 0) uniform sampler2D uFocus;
		layout (binding = 1) uniform sampler2D uLoseFocus;
		layout (binding = 2) uniform sampler2D uPosition;
		layout (binding = 3) uniform Params{
			vec2 screenFocusPoint;
			float near;
			float far;
		}params;

		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;

		void main(){
			outFragColor = vec4(1);
			vec4 position		= texture(uPosition, vUV);
			vec4 focusColor     = texture(uFocus,   vUV);
			vec4 LoseFocusColor = texture(uLoseFocus, vUV);
			vec4 focusPoint		= texture(uPosition, params.screenFocusPoint);
			float blur			= smoothstep(params.near, params.far, length(position - focusPoint));
			outFragColor		= mix(focusColor, LoseFocusColor, blur);
		}
		)");

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

void QDepthOfFieldRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	if (sigUpdateParams.receive()) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(Params), &mParams);
		cmdBuffer->resourceUpdate(batch);
	}
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
