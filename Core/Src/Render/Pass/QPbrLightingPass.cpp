#include "Render/Pass/QPbrLightingPass.h"

static float CubeData[] = { // Y up, front = CCW
		// positions	     
		 1.0f, -1.0f, -1.0f, 	//+x
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f, 	//-x	
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f, 	//+y
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f, 	//-y
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,

		-1.0f, -1.0f,  1.0f, 	//+z
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f, 	//-z
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

};

void QPbrLightingPass::resizeAndLinkNode(const QSize& size) {
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_FragColor(mRT.colorAttachment.get());

	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	mSampler->create();

	mPbrUniformBlock.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(PbrUniformBlock)));
	mPbrUniformBlock->create();

	mPbrBindings.reset(mRhi->newShaderResourceBindings());
	mPbrBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Albedo(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Position(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(2,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Normal(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(3,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Metallic(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(4,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Roughness(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(5,QRhiShaderResourceBinding::FragmentStage, getTextureIn_SpecularCube(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(6,QRhiShaderResourceBinding::FragmentStage, getTextureIn_IrradianceCube(), mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(7,QRhiShaderResourceBinding::FragmentStage, getTextureIn_BrdfLut(), mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(8,QRhiShaderResourceBinding::FragmentStage, mPbrUniformBlock.get())
	});
	mPbrBindings->create();

	mSkyboxUniformBlock.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(SkyboxUniformBlock)));
	mSkyboxUniformBlock->create();

	mSkyboxBindings.reset(mRhi->newShaderResourceBindings());
	mSkyboxBindings->setBindings({
		QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage, mSkyboxUniformBlock.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, getTextureIn_SkyCube(), mSampler.get()),
	});
	mSkyboxBindings->create();
}

void QPbrLightingPass::compile() {
	mSkyboxVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(CubeData)));
	mSkyboxVertexBuffer->create();
	mSkyboxPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
	mSkyboxPipeline->setTargetBlends({ blendState });
	mSkyboxPipeline->setSampleCount(mRT.renderTarget->sampleCount());
	QShader skyboxVs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 450
		layout(location = 0) in vec3 inPosition;
		layout(location = 0) out vec3 vPosition;
		layout(binding = 0) uniform UniformBlock{
			mat4 MVP;
		}UBO;
		void main(){
			vPosition = inPosition;
			gl_Position = UBO.MVP * vec4(inPosition,1.0f);
			gl_Position.z = 1.0f;
		}
	)");
	QShader skyboxFs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout(location = 0) in vec3 vPosition;
		layout(binding  = 1) uniform samplerCube uSkyCube;
		layout(location = 0) out vec4 outFragColor;
		void main(){
			outFragColor = texture(uSkyCube,vPosition);
		}
	)");
	mSkyboxPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, skyboxVs },
		{ QRhiShaderStage::Fragment, skyboxFs }
	});
	QRhiVertexInputLayout skyboxInputLayout;
	skyboxInputLayout.setAttributes({
		QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3,0),
	});
	skyboxInputLayout.setBindings({
		QRhiVertexInputBinding(sizeof(QVector3D))
	});
	mSkyboxPipeline->setVertexInputLayout(skyboxInputLayout);
	mSkyboxPipeline->setShaderResourceBindings(mSkyboxBindings.get());
	mSkyboxPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
	mSkyboxPipeline->create();

	mPbrPipeline.reset(mRhi->newGraphicsPipeline());
	blendState.srcColor = QRhiGraphicsPipeline::SrcAlpha;
	blendState.enable = true;
	mPbrPipeline->setTargetBlends({ blendState });
	mPbrPipeline->setSampleCount(mRT.renderTarget->sampleCount());
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
		layout(binding = 0) uniform sampler2D albedoTexture;
		layout(binding = 1) uniform sampler2D positionTexture;
		layout(binding = 2) uniform sampler2D normalTexture;
		layout(binding = 3) uniform sampler2D metalnessTexture;
		layout(binding = 4) uniform sampler2D roughnessTexture;
		layout(binding = 5) uniform samplerCube specularTexture;
		layout(binding = 6) uniform samplerCube irradianceTexture;
		layout(binding = 7) uniform sampler2D specularBRDF_LUT;
		layout(binding = 8) uniform UniformBlock{
			vec3 eyePosition;
		}UBO;

		layout(location = 0) in vec2 vUV;
		layout(location = 0) out vec4 outFragColor;

		const float PI = 3.141592;
		const float Epsilon = 0.00001;
		const int NumLights = 3;
		const vec3 Fdielectric = vec3(0.04);

		float ndfGGX(float cosLh, float roughness){
			float alpha   = roughness * roughness;
			float alphaSq = alpha * alpha;

			float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
			return alphaSq / (PI * denom * denom);
		}

		float gaSchlickG1(float cosTheta, float k){
			return cosTheta / (cosTheta * (1.0 - k) + k);
		}

		float gaSchlickGGX(float cosLi, float cosLo, float roughness){
			float r = roughness + 1.0;
			float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
			return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
		}
		
		vec3 fresnelSchlick(vec3 F0, float cosTheta){
			return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
		}

		void main(){
			vec4 albedo = texture(albedoTexture, vUV);
			vec3 position = texture(positionTexture, vUV).rgb;
			float metalness = texture(metalnessTexture, vUV).r;
			float roughness = texture(roughnessTexture, vUV).r;
			vec3 Lo = normalize(UBO.eyePosition - position);
			vec3 N = texture(normalTexture, vUV).rgb;
			float cosLo = max(0.0, dot(N, Lo));
			vec3 Lr = 2.0 * cosLo * N - Lo;
			vec3 F0 = mix(Fdielectric, albedo.rgb, metalness);

			vec3 directLighting = vec3(0);

			vec3 ambientLighting;
			{
				vec3 irradiance = texture(irradianceTexture, N).rgb;
				vec3 F = fresnelSchlick(F0, cosLo);
				vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);
				vec3 diffuseIBL = kd * albedo.rgb * irradiance;
				int specularTextureLevels = textureQueryLevels(specularTexture);
				vec3 specularIrradiance = textureLod(specularTexture, Lr, roughness * specularTextureLevels).rgb;
				vec2 specularBRDF = texture(specularBRDF_LUT, vec2(cosLo, roughness)).rg;
				vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
				ambientLighting = specularIBL + diffuseIBL;
			}
			outFragColor = vec4(ambientLighting , albedo.a);
		}
	)");

	mPbrPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	QRhiVertexInputLayout inputLayout;
	mPbrPipeline->setVertexInputLayout(inputLayout);
	mPbrPipeline->setShaderResourceBindings(mPbrBindings.get());
	mPbrPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
	mPbrPipeline->create();

	sigUploadSkyboxVertics.request();
}

void QPbrLightingPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	if (sigUploadSkyboxVertics.receive()) {
		batch->uploadStaticBuffer(mSkyboxVertexBuffer.get(), CubeData);
	}
	SkyboxUniformBlock skyUb;
	QMatrix4x4 MVP = mRenderer->getCamera()->getMatrixClipWithCorr(mRhi) * mRenderer->getCamera()->getMatrixView();
	MVP.scale(2000);
	skyUb.MVP = MVP.toGenericMatrix<4,4>();
	batch->updateDynamicBuffer(mSkyboxUniformBlock.get(), 0, sizeof(SkyboxUniformBlock), &skyUb);

	PbrUniformBlock pbrUb;
	pbrUb.eyePosition = mRenderer->getCamera()->getPosition();
	batch->updateDynamicBuffer(mPbrUniformBlock.get(),0,sizeof(PbrUniformBlock), &pbrUb);

	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	
	cmdBuffer->setGraphicsPipeline(mSkyboxPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mSkyboxBindings.get());
	const QRhiCommandBuffer::VertexInput VertexInput(mSkyboxVertexBuffer.get(), 0);
	cmdBuffer->setVertexInput(0, 1, &VertexInput);
	cmdBuffer->draw(36);

	cmdBuffer->setGraphicsPipeline(mPbrPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mPbrBindings.get());
	cmdBuffer->draw(4);

	cmdBuffer->endPass();
}
