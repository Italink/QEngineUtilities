#include "Render/RenderGraph//PassBuilder/QBlurPassBuilder.h"

QBlurPassBuilder::QBlurPassBuilder()
{
	mDownSampleFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (binding = 0) uniform sampler2D uTexture;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		void main(){
			outFragColor = texture(uTexture,vUV);
		}
	)");

	mBlurHFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
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

	mBlurVFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
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
}

void QBlurPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mBlurRT[0].colorAttachment, "BlurTextureH", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize() / mInput._DownSampleCount, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mBlurRT[0].renderTarget, "BlurRenderTargetH", { mBlurRT[0].colorAttachment.get() });
	
	builder.setupTexture(mBlurRT[1].colorAttachment, "BlurTextureV", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize() / mInput._DownSampleCount, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mBlurRT[1].renderTarget, "BlurRenderTargetV", { mBlurRT[1].colorAttachment.get() });

	builder.setupSampler(mSampler,"BlurSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupBuffer(mUniformBuffer, "BlurKernel", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(BlurKernel));

	builder.setupShaderResourceBindings(mBindingsDownSample, "BlurBindingsDownSample", {
		QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, mInput._BaseColorTexture.get(), mSampler.get())
	});

	builder.setupShaderResourceBindings(mBindingsH, "BlurBindingsH", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mBlurRT[0].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	builder.setupShaderResourceBindings(mBindingsV, "BlurBindingsV", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mBlurRT[1].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	QRhiGraphicsPipelineState PSO;
	QRhiGraphicsPipeline::TargetBlend blendStateDownSample;
	blendStateDownSample.enable = false;
	PSO.targetBlends = { blendStateDownSample };
	PSO.sampleCount = mBlurRT[0].renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindingsDownSample.get();
	PSO.renderPassDesc = mBlurRT[0].renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mDownSampleFS }
	};
	builder.setupGraphicsPipeline(mPipelineDownSample, "BlurDownSamplePipeline", PSO);

	PSO.shaderResourceBindings = mBindingsH.get();
	PSO.sampleCount = mBlurRT[1].renderTarget->sampleCount();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mBlurHFS }
	};
	builder.setupGraphicsPipeline(mPipelineH, "BlurHPipeline", PSO);

	PSO.shaderResourceBindings = mBindingsV.get();
	PSO.renderPassDesc = mBlurRT[0].renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mBlurVFS }
	};
	builder.setupGraphicsPipeline(mPipelineV, "BlurVPipeline", PSO);

	mOutput.BlurResult = mBlurRT[1].colorAttachment;
}

void QBlurPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	cmdBuffer->beginPass(mBlurRT[0].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipelineDownSample.get());
	cmdBuffer->setShaderResources(mBindingsDownSample.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mBlurRT[0].renderTarget->pixelSize().width(), mBlurRT[0].renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();

	if (mInput._BlurSize != mKernel.size) {
		refreshKernel();
		QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(BlurKernel), &mKernel);
		cmdBuffer->resourceUpdate(batch);
	}

	for (int i = 0; i < mInput._BlurIterations; i++) {
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

void QBlurPassBuilder::refreshKernel()
{
	if (mInput._BlurSize <= 0 || mInput._BlurSize == mKernel.size || mInput._BlurSize >= std::size(mKernel.weight))
		return;
	mKernel.size = mInput._BlurSize;
	float sum = 1, s = 1;
	mKernel.weight[mInput._BlurSize - 1] = 1;
	for (int i = mInput._BlurSize - 2; i >= 0; i--) {
		mKernel.weight[i] = (mKernel.weight[i + 1] + s);
		++s;
		sum += mKernel.weight[i] * 2;
	}
	mKernel.weight[0] /= sum / 2;
	for (int i = 1; i < mInput._BlurSize; i++) {
		mKernel.weight[i] /= sum;
	}
}
