#include "Render/Pass/QOutliningRenderPass.h"

QOutliningRenderPass::QOutliningRenderPass() {
}

void QOutliningRenderPass::resizeAndLinkNode(const QSize& size) {
	auto baseColorTexture = getTextureIn_BaseColor();
	auto positionTexture = getTextureIn_Position();
	mOutliningRT.colorAttachment.reset(mRhi->newTexture(baseColorTexture->format(), baseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mOutliningRT.colorAttachment->create();
	mOutliningRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mOutliningRT.colorAttachment.get() }));
	renderPassDesc.reset(mOutliningRT.renderTarget->newCompatibleRenderPassDescriptor());

	mOutliningRT.renderTarget->setRenderPassDescriptor(renderPassDesc.get());
	mOutliningRT.renderTarget->create();
	
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge)
	);
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
	mUniformBuffer->create();

	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,baseColorTexture,mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,positionTexture,mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(2,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});
	mBindings->create();

	registerTextureOut_Result(mOutliningRT.colorAttachment.get());

	sigUpdateParams.request();
}

void QOutliningRenderPass::compile() {
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
	mPipeline->setTargetBlends({ blendState });
	mPipeline->setSampleCount(1);
	mPipeline->setDepthTest(false);
	mPipeline->setDepthWrite(false);

	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 450
		layout (location = 0) out vec2 vUV;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
#if Y_UP_IN_NDC
			gl_Position.y = - gl_Position.y;
#endif 
		})"
		, QShaderDefinitions()
		.addDefinition("Y_UP_IN_NDC", mRhi->isYUpInNDC())
	);

	QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
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
	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	QRhiVertexInputLayout inputLayout;
	mPipeline->setVertexInputLayout(inputLayout);
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(renderPassDesc.get());
	mPipeline->create();
}

void QOutliningRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QMatrix4x4 VP = mRenderer->getCamera()->getProjectionMatrixWithCorr(mRhi) * mRenderer->getCamera()->getViewMatrix();
	mParams.VP = VP.toGenericMatrix<4, 4>();
	mParams.FarNear = mRenderer->getCamera()->getFarPlane() / mRenderer->getCamera()->getNearPlane();
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(Params), &mParams);
	cmdBuffer->resourceUpdate(batch);

	cmdBuffer->beginPass(mOutliningRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mOutliningRT.renderTarget->pixelSize().width(), mOutliningRT.renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
