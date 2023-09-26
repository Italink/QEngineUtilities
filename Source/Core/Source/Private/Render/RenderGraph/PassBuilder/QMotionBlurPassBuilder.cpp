#include "QMotionBlurPassBuilder.h"
#include "IRenderer.h"

QMotionBlurPassBuilder::QMotionBlurPassBuilder()
{
	mMontionBlurFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout(location = 0) in vec2 vUV;
		layout(location = 0) out vec4 outFragColor;

		layout(binding = 0) uniform sampler2D uBaseColor;
		layout(binding = 1) uniform sampler2D uPosition;
		layout(binding = 2) uniform MontionBlurParams {
			mat4 previousViewToWorld;
			mat4 worldToView;
			mat4 projection;
			int size;
			float separation;
		}params;

		void main() {
			vec2 texSize = textureSize(uBaseColor, 0).xy;
			vec2 vUV = gl_FragCoord.xy / texSize;

			outFragColor = texture(uBaseColor, vUV);

			vec4 position1 = texture(uPosition, vUV);

			if (params.size <= 0 || params.separation <= 0.0) { return; }

			vec4 position0 = params.worldToView * params.previousViewToWorld * position1;

			position0 = params.projection * position0;
			position0.xyz /= position0.w;
			position0.xy = position0.xy * 0.5 + 0.5;

			position1 = params.projection * position1;
			position1.xyz /= position1.w;
			position1.xy = position1.xy * 0.5 + 0.5;

			vec2 direction = position1.xy - position0.xy;

			direction.xy *= params.separation;

			vec2 forward = vUV;
			vec2 backward = vUV;

			float count = 1.0;

			for (int i = 0; i < params.size; ++i) {
				forward += direction;
				backward -= direction;

				outFragColor += texture(uBaseColor, forward);
				outFragColor += texture(uBaseColor, backward);

				count += 2.0;
			}
			outFragColor /= count;
		}
	)");
}

void QMotionBlurPassBuilder::setup(QRenderGraphBuilder& builder)
{
	mRenderer = builder.renderer();
	builder.setupTexture(mRT.colorAttachment, "MotionBlurTexture", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "MotionBlurRenderTarget", { mRT.colorAttachment.get() });

	builder.setupBuffer(mUniformBuffer, "MotionBlurUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));

	builder.setupSampler(mSampler, "MotionBlurSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindings, "MotionBlurBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, mInput._BaseColorTexture.get(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,  mInput._PositionTexture.get(), mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(2,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get()),
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.fullScreenVS() },
		{ QRhiShaderStage::Fragment, mMontionBlurFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "MotionBlurPipeline", PSO);

	mOutput.MotionBlurReslut = mRT.colorAttachment;
}

void QMotionBlurPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	QMatrix4x4 view = mRenderer->getCamera()->getViewMatrix();
	mParams.projection = mRenderer->getCamera()->getProjectionMatrixWithCorr(cmdBuffer->rhi()).toGenericMatrix<4, 4>();
	mParams.worldToView = view.toGenericMatrix<4, 4>();
	mParams.previousViewToWorld = view.inverted().toGenericMatrix<4, 4>();
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &mParams);
	cmdBuffer->resourceUpdate(batch);

	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();

}
