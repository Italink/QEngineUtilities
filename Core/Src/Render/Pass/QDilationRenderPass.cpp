#include "Render/Pass/QDilationRenderPass.h"

QDilationRenderPass::QDilationRenderPass() {
}

void QDilationRenderPass::setDilationSize(int size) {
	if (size <= 0)
		return;
	mParams.size = size;
	sigUpdateParams.request();
}

void QDilationRenderPass::setDilationSeparation(float val) {
	mParams.separation = val;
	sigUpdateParams.request();
}

void QDilationRenderPass::setMinThreshold(float val) {
	mParams.minThreshold = val;
	sigUpdateParams.request();
}

void QDilationRenderPass::setMaxThreshold(float val) {
	mParams.maxThreshold = val;
	sigUpdateParams.request();
}

void QDilationRenderPass::resizeAndLinkNode(const QSize& size) {
	mSrcTexture = getTextureIn_Src();
	for (int i = 0; i < 2; i++) {
		mDilationRT[i].colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, mSrcTexture->pixelSize() , 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mDilationRT[i].colorAttachment->create();
		mDilationRT[i].renderTarget.reset(mRhi->newTextureRenderTarget({ mDilationRT[i].colorAttachment.get() }));
	}
	renderPassDesc.reset(mDilationRT[0].renderTarget->newCompatibleRenderPassDescriptor());

	for (int i = 0; i < 2; i++) {
		mDilationRT[i].renderTarget->setRenderPassDescriptor(renderPassDesc.get());
		mDilationRT[i].renderTarget->create();
	}
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
	mUniformBuffer->create();

	mBindingsH.reset(mRhi->newShaderResourceBindings());
	mBindingsH->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mDilationRT[0].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
		});
	mBindingsH->create();

	mBindingsV.reset(mRhi->newShaderResourceBindings());
	mBindingsV->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mDilationRT[1].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
		});

	mBindingsV->create();

	registerTextureOut_Result(mDilationRT[0].colorAttachment.get());

	sigUpdateParams.request();
}

void QDilationRenderPass::compile() {
	mPipelineH.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
	mPipelineH->setTargetBlends({ blendState });
	mPipelineH->setSampleCount(1);
	mPipelineH->setDepthTest(false);
	mPipelineH->setDepthWrite(false);

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
	QShader fsH = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		layout (binding = 0) uniform sampler2D uTexture;

		layout (binding = 1 ) uniform Params{
			int size;
			float separation;
			float minThreshold;
			float maxThreshold;
		}params;

		void main(){
			vec2 tex_offset = 1.0 / textureSize(uTexture, 0);  // gets size of single texel
			vec4 centerColor = texture(uTexture, vUV);

			float maxGreyScale = 0.0f;
			vec4 maxColor = centerColor;

			for(int i = 1; i < params.size; ++i){
				vec4 leftColor  = texture(uTexture, vUV - vec2(tex_offset.x * i, 0.0) * params.separation);
				float leftGreyScale = dot(leftColor.rgb, vec3(0.3, 0.59, 0.11));
				maxColor	 = leftGreyScale > maxGreyScale ? leftColor : maxColor;
				maxGreyScale = leftGreyScale > maxGreyScale ? leftGreyScale : maxGreyScale;
				
				vec4 rightColor = texture(uTexture, vUV + vec2(tex_offset.x * i, 0.0) * params.separation);
				float rightGreyScale = dot(rightColor.rgb, vec3(0.3, 0.59, 0.11));
				maxColor	 = rightGreyScale > maxGreyScale ? rightColor : maxColor;
				maxGreyScale = rightGreyScale > maxGreyScale ? rightGreyScale : maxGreyScale;
			}
			outFragColor = maxColor;
		}
	)");
	mPipelineH->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fsH }
	});

	QRhiVertexInputLayout inputLayout;
	mPipelineH->setVertexInputLayout(inputLayout);
	mPipelineH->setShaderResourceBindings(mBindingsH.get());
	mPipelineH->setRenderPassDescriptor(renderPassDesc.get());
	mPipelineH->create();

	mPipelineV.reset(mRhi->newGraphicsPipeline());
	mPipelineV->setTargetBlends({ blendState });
	mPipelineV->setSampleCount(1);
	mPipelineV->setDepthTest(false);
	mPipelineV->setDepthWrite(false);
	QShader fsV = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		layout (binding = 0) uniform sampler2D uTexture;
		layout (binding = 1 ) uniform Params{
			int size;
			float separation;
			float minThreshold;
			float maxThreshold;
		}params;

		void main(){
			vec2 tex_offset = 1.0 / textureSize(uTexture, 0);  // gets size of single texel
			vec4 centerColor = texture(uTexture, vUV);

			float maxGreyScale = 0.0f;
			vec4 maxColor = centerColor;

			for(int i = 1; i < params.size; ++i){
				vec4 bottomColor = texture(uTexture, vUV - vec2(0.0f,tex_offset.x * i) * params.separation);
				float bottomGreyScale = dot(bottomColor.rgb, vec3(0.3, 0.59, 0.11));
				maxColor	 = bottomGreyScale > maxGreyScale ? bottomColor : maxColor;
				maxGreyScale = bottomGreyScale > maxGreyScale ? bottomGreyScale : maxGreyScale;

				vec4 topColor  = texture(uTexture, vUV + vec2(0.0f,tex_offset.x * i) * params.separation);
				float topGreyScale = dot(topColor.rgb, vec3(0.3, 0.59, 0.11));
				maxColor	 = topGreyScale > maxGreyScale ? topColor : maxColor;
				maxGreyScale = topGreyScale > maxGreyScale ? topGreyScale : maxGreyScale;
			}
			outFragColor = vec4(mix(centerColor.rgb, maxColor.rgb, smoothstep(params.minThreshold, params.maxThreshold, maxGreyScale)),1.0f);
		}
	)");
	mPipelineV->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fsV }
	});
	mPipelineV->setVertexInputLayout(inputLayout);
	mPipelineV->setShaderResourceBindings(mBindingsV.get());
	mPipelineV->setRenderPassDescriptor(renderPassDesc.get());
	mPipelineV->create();
}

void QDilationRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->copyTexture(mDilationRT[0].colorAttachment.get(), getTextureIn_Src());
	if (sigUpdateParams.ensure()) {
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(Params), &mParams);
	}
	cmdBuffer->beginPass(mDilationRT[1].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	cmdBuffer->setGraphicsPipeline(mPipelineH.get());
	cmdBuffer->setShaderResources(mBindingsH.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mDilationRT[1].renderTarget->pixelSize().width(), mDilationRT[1].renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();

	cmdBuffer->beginPass(mDilationRT[0].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipelineV.get());
	cmdBuffer->setShaderResources(mBindingsV.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mDilationRT[0].renderTarget->pixelSize().width(), mDilationRT[0].renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
	
}
