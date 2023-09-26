#include "QOutliningPassBuilder.h"
#include "IRenderer.h"

QOutliningPassBuilder::QOutliningPassBuilder()
{
	mOutliningFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		layout (binding = 0) uniform sampler2D uBaseColor;
		layout (binding = 1) uniform sampler2D uPosition;
		layout (binding = 2) uniform Params{
			mat4 VP;
			float MinSeparation;
			float MaxSeparation;
			float MinDistance;
			float MaxDistance;
			float FarNear;
			int Radius;
			vec4 ColorModifier;
		}params;

		void main(){
			vec2 texOffset = 1.0 / textureSize(uBaseColor, 0);
			vec4 baseColor = texture(uBaseColor, vUV);
			vec4 screenPosition = params.VP * texture(uPosition, vUV);
			float depth = screenPosition.z/screenPosition.w;
			vec2 separation = mix(params.MaxSeparation, params.MinSeparation, depth) * texOffset;
			float mx = 0.0f;
			for (int i = -params.Radius; i <= params.Radius; ++i) {
				for (int j = -params.Radius; j <= params.Radius; ++j) {
					vec2 currUV = vUV + vec2(i,j) * separation;
					vec4 currScreenPosition = params.VP * texture(uPosition, currUV);
					float currDepth = currScreenPosition.z/currScreenPosition.w;
					mx = max(mx, abs(depth - currDepth));
				}
			}
			float diff = smoothstep(params.MinDistance, params.MaxDistance, mx * params.FarNear);
			vec3 lineColor = baseColor.rgb * params.ColorModifier.rgb;
			outFragColor = vec4(mix(baseColor.rgb, lineColor, diff ),baseColor.a);
		}
	)");
}

void QOutliningPassBuilder::setup(QRenderGraphBuilder& builder)
{
	mRenderer = builder.renderer();
	builder.setupTexture(mRT.colorAttachment, "OutliningTexture", QRhiTexture::RGBA8, mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "OutliningRenderTarget", { mRT.colorAttachment.get() });

	builder.setupBuffer(mUniformBuffer, "OutlinlingUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));

	builder.setupSampler(mSampler, "OutliningSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindings, "OutliningBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._BaseColorTexture.get(),mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,mInput._PositionTexture.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(2,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.fullScreenVS() },
		{ QRhiShaderStage::Fragment, mOutliningFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "OutliningPipeline", PSO);

	mOutput.OutliningReslut = mRT.colorAttachment;
}

void QOutliningPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	QMatrix4x4 VP = mRenderer->getCamera()->getProjectionMatrixWithCorr(cmdBuffer->rhi()) * mRenderer->getCamera()->getViewMatrix();
	mParams.VP = VP.toGenericMatrix<4, 4>();
	mParams.FarNear = mRenderer->getCamera()->getFarPlane() / mRenderer->getCamera()->getNearPlane();
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
