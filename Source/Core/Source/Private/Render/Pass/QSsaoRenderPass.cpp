#include "Render/Pass/QSsaoRenderPass.h"
#include "QRandomGenerator"

QSsaoRenderPass::QSsaoRenderPass() {
	setupSampleSize(64);
}

float lerp(float a, float b, float f) {
	return a + f * (b - a);
}

QSsaoRenderPass* QSsaoRenderPass::setupSampleSize(int size) {
	if (size <= 0 || size == mSsaoState.size || size >= std::size(mSsaoState.samples))
		return this;
	mSsaoState.size = size;
	QRandomGenerator generator;
	for (unsigned int i = 0; i < size; ++i) {
		QVector3D sample(
			generator.generateDouble() * 2.0 - 1.0,
			generator.generateDouble() * 2.0 - 1.0,
			generator.generateDouble()
		);
		sample.normalize();
		float scale = i / (float) size;
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
	sigUpdateSsaoState.request();
	return this;
}

QSsaoRenderPass* QSsaoRenderPass::setupRadius(float var) {
	mSsaoState.radius = var;
	sigUpdateSsaoState.request();
	return this;
}

QSsaoRenderPass* QSsaoRenderPass::setupBias(float var) {
	mSsaoState.bias = var;
	sigUpdateSsaoState.request();
	return this;
}

void QSsaoRenderPass::resizeAndLinkNode(const QSize& size) {
	auto positionTexture = getTextureIn_Position();
	auto normalTexture = getTextureIn_Normal();
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, positionTexture->pixelSize() , 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_Result(mRT.colorAttachment.get());

	mSampler.reset(mRhi->newSampler(QRhiSampler::Nearest,
		QRhiSampler::Nearest,
		QRhiSampler::Nearest,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(SsaoState)));
	mUniformBuffer->create();

	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,positionTexture,mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,normalTexture,mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(2,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});
	mBindings->create();
	sigUpdateSsaoState.request();
}

void QSsaoRenderPass::compile() {
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
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
		layout (binding = 0) uniform sampler2D uPosition;
		layout (binding = 1) uniform sampler2D uNormal;
		layout (binding = 2) uniform SsaoState{
			mat4 VP;
			float radius;
			float bias;
			int size;
			vec4 samples[128];
			vec4 noise[16];
		}ssaoState;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
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
			for (int i = 0; i < ssaoState.size; ++i) {
				vec3 samplePos = TBN * ssaoState.samples[i].xyz;
				samplePos = position + samplePos * ssaoState.radius;

				vec4 offset = ssaoState.VP * vec4(samplePos, 1.0);
				offset.xyz /= offset.w;
				offset.xyz  = offset.xyz * 0.5 + 0.5; 

				float sampleDepth = texture(uPosition, offset.xy).z; 
				float rangeCheck = smoothstep(0.0, 1.0, ssaoState.radius / abs(position.z - sampleDepth));
				occlusion       += (sampleDepth >= samplePos.z + ssaoState.bias ? 1.0 : 0.0) * rangeCheck;  
			}
			occlusion = 1.0 - (occlusion / ssaoState.size);
			outFragColor = vec4(occlusion,occlusion,occlusion,1.0f);  
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

void QSsaoRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QMatrix4x4 VP = mRenderer->getCamera()->getProjectionMatrixWithCorr(mRhi) * mRenderer->getCamera()->getViewMatrix();
	mSsaoState.VP = VP.toGenericMatrix<4, 4>();
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	if (sigUpdateSsaoState.ensure()) {
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(SsaoState), &mSsaoState);
	}
	else {
		batch->updateDynamicBuffer(mUniformBuffer.get(), offsetof(SsaoState, VP), sizeof(float) * 16, VP.data());
	}
	cmdBuffer->resourceUpdate(batch);
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
