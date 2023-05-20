#include "Render/Pass/QBlurRenderPass.h"

QBlurRenderPass::QBlurRenderPass() {
	setBlurSize(20);
	setBlurIter(2);
}

void QBlurRenderPass::setBlurSize(int size) {
	if (size <= 0 || size == mParams.size || size >= std::size(mParams.weight))
		return;
	mParams.size = size;
	float sum = 1, s = 1;
	mParams.weight[size - 1] = 1;
	for (int i = size - 2; i >= 0; i--) {
		mParams.weight[i] = (mParams.weight[i + 1] + s);
		++s;
		sum += mParams.weight[i] * 2;
	}
	mParams.weight[0] /= sum / 2;
	for (int i = 1; i < size; i++) {
		mParams.weight[i] /= sum;
	}
	sigUpdateParams.request();
}

void QBlurRenderPass::setBlurIter(int val) {
	mBlurIter = val;
}

void QBlurRenderPass::setDownSample(int val) {
	mDownSampleCount = val;
	if(mRenderer)
		mRenderer->requestComplie();
}

void QBlurRenderPass::resizeAndLinkNode(const QSize& size) {
	mSrcTexture = getTextureIn_Src();
	for (int i = 0; i < 2; i++) {
		mBlurRT[i].colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, mSrcTexture->pixelSize() / mDownSampleCount, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mBlurRT[i].colorAttachment->create();
		mBlurRT[i].renderTarget.reset(mRhi->newTextureRenderTarget({ mBlurRT[i].colorAttachment.get() }));
	}
	renderPassDesc.reset(mBlurRT[0].renderTarget->newCompatibleRenderPassDescriptor());

	for (int i = 0; i < 2; i++) {
		mBlurRT[i].renderTarget->setRenderPassDescriptor(renderPassDesc.get());
		mBlurRT[i].renderTarget->create();
	}
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge)
	);
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
	mUniformBuffer->create();

	mBindingsDownSample.reset(mRhi->newShaderResourceBindings());
	mBindingsDownSample->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, mSrcTexture, mSampler.get())
	});
	mBindingsDownSample->create();

	mBindingsH.reset(mRhi->newShaderResourceBindings());
	mBindingsH->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mBlurRT[0].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});
	mBindingsH->create();

	mBindingsV.reset(mRhi->newShaderResourceBindings());
	mBindingsV->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mBlurRT[1].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	mBindingsV->create();

	registerTextureOut_Result(mBlurRT[0].colorAttachment.get());

	sigUpdateParams.request();
}

void QBlurRenderPass::compile() {
	mPipelineDownSample.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendStateDownSample;
	blendStateDownSample.enable = false;
	mPipelineDownSample->setTargetBlends({ blendStateDownSample });
	mPipelineDownSample->setSampleCount(mBlurRT[0].renderTarget->sampleCount());

	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 450
		layout (location = 0) out vec2 vUV;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
#if Y_UP_IN_NDC
			vUV.y = 1 - vUV.y;
#endif 
		})"
		, QShaderDefinitions()
		.addDefinition("Y_UP_IN_NDC", mRhi->isYUpInNDC())
	);

	QShader fsDownSample = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (binding = 0) uniform sampler2D uTexture;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		void main(){
			outFragColor = texture(uTexture,vUV);
		}
	)");
	mPipelineDownSample->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fsDownSample }
	});
	QRhiVertexInputLayout inputLayoutDownSample;
	mPipelineDownSample->setVertexInputLayout(inputLayoutDownSample);
	mPipelineDownSample->setShaderResourceBindings(mBindingsDownSample.get());
	mPipelineDownSample->setRenderPassDescriptor(mBlurRT[0].renderTarget->renderPassDescriptor());
	mPipelineDownSample->create();

	//-------------------------------------------------------------
	
	mPipelineH.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
	mPipelineH->setTargetBlends({ blendState });
	mPipelineH->setSampleCount(1);
	mPipelineH->setDepthTest(false);
	mPipelineH->setDepthWrite(false);
	QShader fsH = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		layout (binding = 0) uniform sampler2D uTexture;

		layout (binding = 1 ) uniform Params{
			int size;
			vec4 weight[50];
		}params;

		void main(){
			vec2 tex_offset = 1.0 / textureSize(uTexture, 0); // gets size of single texel
			vec4 raw = texture(uTexture, vUV);
			vec4 result = raw * params.weight[0][0]; // current fragment's contribution
			for(int i = 1; i < params.size; ++i){
				const float weight = params.weight[i/4][i%4];
				result += texture(uTexture, vUV + vec2(tex_offset.x * i, 0.0)) * weight;
				result += texture(uTexture, vUV - vec2(tex_offset.x * i, 0.0)) * weight;
			}
			outFragColor = result;
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

	//-------------------------------------------------------------

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
		vec4 weight[50];
	}params;

	void main(){
		vec2 tex_offset = 1.0 / textureSize(uTexture, 0); // gets size of single texel
		vec4 raw = texture(uTexture, vUV);
		vec4 result = raw * params.weight[0][0]; // current fragment's contribution
		for(int i = 1; i < params.size; ++i){
			const float weight = params.weight[i/4][i%4];
			result += texture(uTexture, vUV + vec2(0.0,tex_offset.y * i)) * weight;
			result += texture(uTexture, vUV - vec2(0.0,tex_offset.y * i)) * weight;
		}
		outFragColor = result;
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

void QBlurRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	cmdBuffer->beginPass(mBlurRT[0].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipelineDownSample.get());
	cmdBuffer->setShaderResources(mBindingsDownSample.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mBlurRT[0].renderTarget->pixelSize().width(), mBlurRT[0].renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();

	if (sigUpdateParams.ensure()) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(Params), &mParams);
		cmdBuffer->resourceUpdate(batch);
	}

	for (int i = 0; i < mBlurIter; i++) {
		cmdBuffer->beginPass(mBlurRT[1].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
		cmdBuffer->setGraphicsPipeline(mPipelineH.get());
		cmdBuffer->setShaderResources(mBindingsH.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mBlurRT[1].renderTarget->pixelSize().width(), mBlurRT[1].renderTarget->pixelSize().height()));
		cmdBuffer->draw(4);
		cmdBuffer->endPass();

		cmdBuffer->beginPass(mBlurRT[0].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
		cmdBuffer->setGraphicsPipeline(mPipelineV.get());
		cmdBuffer->setShaderResources(mBindingsV.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mBlurRT[0].renderTarget->pixelSize().width(), mBlurRT[0].renderTarget->pixelSize().height()));
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
}
