#include "QSsaoPassBuilder.h"
#include "IRenderer.h"
#include <QRandomGenerator>

QSsaoPassBuilder::QSsaoPassBuilder()
{
	mSsaoFS = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
		layout (binding = 0) uniform sampler2D uPosition;
		layout (binding = 1) uniform sampler2D uNormal;
		layout (binding = 2) uniform SsaoState{
			mat4 VP;
			float radius;
			float bias;
			int sampleSize;
			vec4 samples[128];
			vec4 noise[16];
		}ssaoState;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out float outFragColor;
		void main(){
			vec3 position = texture(uPosition, vUV).xyz;
			vec3 normal   = normalize(texture(uNormal, vUV).xyz);
			int  noiseX = int(gl_FragCoord.x - 0.5) % 4;
			int  noiseY = int(gl_FragCoord.y - 0.5) % 4;
			vec3 randomVec  = ssaoState.noise[noiseX + (noiseY * 4)].xyz;
		
			vec3 tangent = normalize(randomVec  - normal * dot(randomVec, normal));
			vec3 bitangent = cross(normal, tangent);
			mat3 TBN = mat3(tangent, bitangent, normal);
		
			float occlusion = 0.0f;
			for (int i = 0; i < ssaoState.sampleSize; ++i) {
				vec3 samplePos = TBN * ssaoState.samples[i].xyz;
				samplePos = position + samplePos * ssaoState.radius;
		
				vec4 offset = ssaoState.VP * vec4(samplePos, 1.0);
				offset.xyz /= offset.w;
				offset.xyz  = offset.xyz * 0.5 + 0.5; 
		
				float sampleDepth = texture(uPosition, offset.xy).z; 
				float rangeCheck = smoothstep(0.0, 1.0, ssaoState.radius / abs(position.z - sampleDepth));
				occlusion       += (sampleDepth >= samplePos.z + ssaoState.bias ? 1.0 : 0.0) * rangeCheck;  
			}
			occlusion = 1.0 - (occlusion / ssaoState.sampleSize);
			outFragColor = occlusion;  
		}
	)");
}

void QSsaoPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mRT.colorAttachment, "SsaoTexture", QRhiTexture::R16, mInput._PositionTexture->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "SsaoRenderTarget", { mRT.colorAttachment.get() });

	builder.setupBuffer(mUniformBuffer, "SsaoUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));

	builder.setupSampler(mSampler, "SsaoSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupShaderResourceBindings(mBindings, "SsaoBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._PositionTexture.get(),mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,mInput._NormalTexture.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(2,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.getFullScreenVS() },
		{ QRhiShaderStage::Fragment, mSsaoFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "SsaoPipeline", PSO);

	mOutput.SsaoResult = mRT.colorAttachment;
}

void QSsaoPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	refreshSsaoState();
	QMatrix4x4 VP = mRenderer->getCamera()->getProjectionMatrixWithCorr() * mRenderer->getCamera()->getViewMatrix();
	mSsaoState.VP = VP.toGenericMatrix<4, 4>();
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &mSsaoState);
	cmdBuffer->resourceUpdate(batch);
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}

float lerp(float a, float b, float f) {
	return a + f * (b - a);
}

void QSsaoPassBuilder::refreshSsaoState()
{
	mSsaoState.bias = mInput._Bias;
	mSsaoState.radius = mInput._Radius;
	if (mInput._SampleSize <= 0 || mInput._SampleSize == mSsaoState.size || mInput._SampleSize >= std::size(mSsaoState.samples))
		return;
	mSsaoState.size = mInput._SampleSize;
	QRandomGenerator generator;
	for (int i = 0; i < mInput._SampleSize; ++i) {
		QVector3D sample(
			generator.generateDouble() * 2.0 - 1.0,
			generator.generateDouble() * 2.0 - 1.0,
			generator.generateDouble()
		);
		sample.normalize();
		float scale = i / (float)mInput._SampleSize;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		mSsaoState.samples[i] = sample.toVector4D();
	}

	for (unsigned int i = 0; i < 16; i++) {
		QVector3D noise(
			generator.generateDouble() * 2.0 - 1.0,
			generator.generateDouble() * 2.0 - 1.0,
			0.0f
		);
		mSsaoState.noise[i] = noise.toVector4D();
	}
}
