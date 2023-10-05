#include "QDilationPassBuilder.h"

QDilationPassBuilder::QDilationPassBuilder()
{
	mDilationFSH = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
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

	mDilationFSV = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
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
}

void QDilationPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mDilationRT[0].colorAttachment, "DilationTextureH", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mDilationRT[0].renderTarget, "DilationRenderTargetH", { mDilationRT[0].colorAttachment.get() });

	builder.setupTexture(mDilationRT[1].colorAttachment, "DilationTextureV", mInput._BaseColorTexture->format(), mInput._BaseColorTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mDilationRT[1].renderTarget, "DilationRenderTargetV", { mDilationRT[1].colorAttachment.get() });

	builder.setupBuffer(mUniformBuffer, "DilationUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));

	builder.setupSampler(mSampler, "DilationSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindingsH, "DilationBindingsH", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mDilationRT[0].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	builder.setupShaderResourceBindings(mBindingsV, "DilationBindingsV", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mDilationRT[1].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = 1;
	PSO.renderPassDesc = mDilationRT[0].renderTarget->renderPassDescriptor();
	PSO.shaderResourceBindings = mBindingsH.get();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mDilationFSH }
	};
	builder.setupGraphicsPipeline(mPipelineH, "DilationPipelineH", PSO);

	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mDilationFSV }
	};
	builder.setupGraphicsPipeline(mPipelineV, "DilationPipelineV", PSO);

	mOutput.DilationReslut = mDilationRT[0].colorAttachment;
}

void QDilationPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	batch->copyTexture(mDilationRT[0].colorAttachment.get(), mInput._BaseColorTexture.get());

	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &mParams);
	
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
