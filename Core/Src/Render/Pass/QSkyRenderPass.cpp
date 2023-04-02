#include "QSkyRenderPass.h"
#include "Asset/AssetUtils.h"

static constexpr int kEnvMapSize = 1024;
static constexpr int kIrradianceMapSize = 32;
static constexpr int kBRDF_LUT_Size = 256;

QSkyRenderPass::QSkyRenderPass() {
	sigInitResource.request();
}

void QSkyRenderPass::setSkyBoxImagePath(const QString& inPath) {
	QImage image;
	if (inPath.endsWith(".hdr", Qt::CaseSensitivity::CaseInsensitive)) {
		QSize size;
		mImageData = AssetUtils::loadHdr(inPath, &size);
		image = QImage((uchar*)mImageData.data(), size.width(), size.height(), QImage::Format_RGBA32FPx4).mirrored();
		sigComputeSkyCube.request();
		bIsEquirectangular = true;
	}
	else {
		image = QImage(inPath).convertToFormat(QImage::Format::Format_RGBA8888);
		bIsEquirectangular = false;
	}
	setSkyBoxImage(image);
}

void QSkyRenderPass::setSkyBoxImage(QImage inImage) {
	mSkyBoxImage = inImage;
	sigUploadEquirectTexture.request();
}

QImage QSkyRenderPass::getSkyBoxImage() const {
	return mSkyBoxImage;
}

void QSkyRenderPass::resizeAndLinkNode(const QSize& size) {
	mEquirectTexture.reset(mRhi->newTexture(QRhiTexture::RGBA32F, mSkyBoxImage.size(), 1));
	mEquirectTexture->create();
	registerTextureOut_Preview(mEquirectTexture.get());
	sigUploadEquirectTexture.request();
}

void QSkyRenderPass::compile() {
	if (!sigInitResource.receive()) {
		return;
	}
	mSkyCube.reset(mRhi->newTexture(QRhiTexture::RGBA32F,  QSize(kEnvMapSize, kEnvMapSize), 1, QRhiTexture::CubeMap | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips | QRhiTexture::UsedWithLoadStore));
	mSkyCube->create();
	registerTextureOut_SkyCube(mSkyCube.get());

	mPrefilteredSpecularCube.reset(mRhi->newTexture(QRhiTexture::RGBA32F, QSize(kEnvMapSize, kEnvMapSize), 1, QRhiTexture::CubeMap |QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips | QRhiTexture::UsedWithLoadStore));
	mPrefilteredSpecularCube->create();
	registerTextureOut_SpecularCube(mPrefilteredSpecularCube.get());

	mDiffuseIrradianceCube.reset(mRhi->newTexture(QRhiTexture::RGBA32F, QSize(kIrradianceMapSize, kIrradianceMapSize), 1, QRhiTexture::CubeMap | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips | QRhiTexture::UsedWithLoadStore));
	mDiffuseIrradianceCube->create();
	registerTextureOut_IrradianceCube(mDiffuseIrradianceCube.get());
	
	mBrdfLut.reset(mRhi->newTexture(QRhiTexture::RGBA32F, QSize(kBRDF_LUT_Size, kBRDF_LUT_Size), 1, QRhiTexture::UsedWithLoadStore));
	mBrdfLut->create();
	registerTextureOut_BrdfLut(mBrdfLut.get());
	
	int levels = mRhi->mipLevelsForSize(mSkyCube->pixelSize());
	mPrefilteredSpecularCubeUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mRhi->ubufAlignment() * (levels - 1)));
	mPrefilteredSpecularCubeUniformBuffer->create();
	
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::Repeat,
		QRhiSampler::Repeat));
	mSampler->create();
	
	mSkyCubePipeline.reset(mRhi->newComputePipeline());
	mSkyCubeBindings.reset(mRhi->newShaderResourceBindings());
	mSkyCubeBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::ComputeStage, mEquirectTexture.get(),mSampler.get()),
		QRhiShaderResourceBinding::imageStore(1, QRhiShaderResourceBinding::ComputeStage,mSkyCube.get(),0),
	});
	mSkyCubeBindings->create();
	mSkyCubePipeline->setShaderResourceBindings(mSkyCubeBindings.get());
	QShader compute = mRhi->newShaderFromCode(QShader::ComputeStage, R"(#version 450
		layout(binding=0) uniform sampler2D inputTexture;
		layout(binding=1, rgba32f) restrict writeonly uniform imageCube outputTexture;
		layout(local_size_x=32, local_size_y=32, local_size_z=1) in;
		const float PI = 3.141592;
		const float TwoPI = 2 * PI;

		vec3 getSamplingVector(){
			vec2 st = gl_GlobalInvocationID.xy/vec2(imageSize(outputTexture));
			vec2 uv = 2.0 * vec2(st.x, 1.0-st.y) - vec2(1.0);
			vec3 ret;
			if(gl_GlobalInvocationID.z == 0)      ret = vec3(1.0,  uv.y, -uv.x);
			else if(gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y,  uv.x);
			else if(gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
			else if(gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
			else if(gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
			else if(gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);
			return normalize(ret);
		}

		void main(void){
			vec3 v = getSamplingVector();
			float phi   = atan(v.z, v.x);
			float theta = acos(v.y);
			vec4 color = texture(inputTexture, vec2(phi/TwoPI, theta/PI));
			imageStore(outputTexture, ivec3(gl_GlobalInvocationID), color);
		}
	)");
	mSkyCubePipeline->setShaderStage({ QRhiShaderStage::Compute,compute });
	mSkyCubePipeline->create();

	mPrefilteredSpecularCubePipeline.reset(mRhi->newComputePipeline());
	mPrefilteredSpecularCubeBindings.reset(mRhi->newShaderResourceBindings());
	mPrefilteredSpecularCubeBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::ComputeStage, mSkyCube.get(),mSampler.get()),
		QRhiShaderResourceBinding::imageStore(1, QRhiShaderResourceBinding::ComputeStage,mPrefilteredSpecularCube.get(),0),
		QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::ComputeStage,mPrefilteredSpecularCubeUniformBuffer.get()),
	});
	mPrefilteredSpecularCubeBindings->create();
	mPrefilteredSpecularCubePipeline->setShaderResourceBindings(mPrefilteredSpecularCubeBindings.get());
	compute = mRhi->newShaderFromCode(QShader::ComputeStage, R"(#version 450
		const float PI = 3.141592;
		const float TwoPI = 2 * PI;
		const float Epsilon = 0.00001;

		const uint NumSamples = 1024;
		const float InvNumSamples = 1.0 / float(NumSamples);
		const int NumMipLevels = 1;

		layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
		layout(binding = 0) uniform samplerCube inputTexture;
		layout(binding = 1, rgba32f) restrict writeonly uniform imageCube outputTexture;
		layout(binding = 2) uniform UniformBlock{
			float roughness;
		} UBO;
		
		#define PARAM_ROUGHNESS UBO.roughness

		float radicalInverse_VdC(uint bits){
			bits = (bits << 16u) | (bits >> 16u);
			bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
			bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
			bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
			bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
			return float(bits) * 2.3283064365386963e-10; // / 0x100000000
		}

		vec2 sampleHammersley(uint i){
			return vec2(i * InvNumSamples, radicalInverse_VdC(i));
		}

		vec3 sampleGGX(float u1, float u2, float roughness){
			float alpha = roughness * roughness;

			float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
			float sinTheta = sqrt(1.0 - cosTheta*cosTheta); // Trig. identity
			float phi = TwoPI * u1;

			return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
		}

		float ndfGGX(float cosLh, float roughness){
			float alpha   = roughness * roughness;
			float alphaSq = alpha * alpha;

			float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
			return alphaSq / (PI * denom * denom);
		}

		vec3 getSamplingVector(){
			vec2 st = gl_GlobalInvocationID.xy/vec2(imageSize(outputTexture));
			vec2 uv = 2.0 * vec2(st.x, 1.0-st.y) - vec2(1.0);

			vec3 ret;

			if(gl_GlobalInvocationID.z == 0)      ret = vec3(1.0,  uv.y, -uv.x);
			else if(gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y,  uv.x);
			else if(gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
			else if(gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
			else if(gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
			else if(gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);
			return normalize(ret);
		}

		void computeBasisVectors(const vec3 N, out vec3 S, out vec3 T){
			T = cross(N, vec3(0.0, 1.0, 0.0));
			T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

			T = normalize(T);
			S = normalize(cross(N, T));
		}

		vec3 tangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T){
			return S * v.x + T * v.y + N * v.z;
		}

		void main(void){
			ivec2 outputSize = imageSize(outputTexture).xy;
			if(gl_GlobalInvocationID.x >= outputSize.x || gl_GlobalInvocationID.y >= outputSize.y) {
				return;
			}
	
			vec2 inputSize = vec2(textureSize(inputTexture, 0));
			float wt = 4.0 * PI / (6 * inputSize.x * inputSize.y);
	
			vec3 N = getSamplingVector();
			vec3 Lo = N;
	
			vec3 S, T;
			computeBasisVectors(N, S, T);

			vec3 color = vec3(0);
			float weight = 0;

			for(uint i=0; i<NumSamples; ++i) {
				vec2 u = sampleHammersley(i);
				vec3 Lh = tangentToWorld(sampleGGX(u.x, u.y, PARAM_ROUGHNESS), N, S, T);

				vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

				float cosLi = dot(N, Li);
				if(cosLi > 0.0) {
					float cosLh = max(dot(N, Lh), 0.0);

					float pdf = ndfGGX(cosLh, PARAM_ROUGHNESS) * 0.25;

					float ws = 1.0 / (NumSamples * pdf);

					float mipLevel = max(0.5 * log2(ws / wt) + 1.0, 0.0);

					color  += textureLod(inputTexture, Li, mipLevel).rgb * cosLi;
					weight += cosLi;
				}
			}
			color /= weight;
			imageStore(outputTexture, ivec3(gl_GlobalInvocationID), vec4(color , 1.0));
		}

	)");
	mPrefilteredSpecularCubePipeline->setShaderStage({ QRhiShaderStage::Compute,compute });
	mPrefilteredSpecularCubePipeline->create();

	mDiffuseIrradiancePipeline.reset(mRhi->newComputePipeline());
	mDiffuseIrradianceBindings.reset(mRhi->newShaderResourceBindings());
	mDiffuseIrradianceBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::ComputeStage, mPrefilteredSpecularCube.get(),mSampler.get()),
		QRhiShaderResourceBinding::imageStore(1, QRhiShaderResourceBinding::ComputeStage,mDiffuseIrradianceCube.get(),0),
	});
	mDiffuseIrradianceBindings->create();
	mDiffuseIrradiancePipeline->setShaderResourceBindings(mDiffuseIrradianceBindings.get());
	compute = mRhi->newShaderFromCode(QShader::ComputeStage, R"(#version 450
		const float PI = 3.141592;
		const float TwoPI = 2 * PI;
		const float Epsilon = 0.00001;

		const uint NumSamples = 64 * 1024;
		const float InvNumSamples = 1.0 / float(NumSamples);

		layout(local_size_x=32, local_size_y=32, local_size_z=1) in;
		layout(binding = 0) uniform samplerCube inputTexture;
		layout(binding = 1, rgba32f) restrict writeonly uniform imageCube outputTexture;

		float radicalInverse_VdC(uint bits){
			bits = (bits << 16u) | (bits >> 16u);
			bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
			bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
			bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
			bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
			return float(bits) * 2.3283064365386963e-10; // / 0x100000000
		}

		vec2 sampleHammersley(uint i){
			return vec2(i * InvNumSamples, radicalInverse_VdC(i));
		}

		vec3 sampleHemisphere(float u1, float u2){
			const float u1p = sqrt(max(0.0, 1.0 - u1*u1));
			return vec3(cos(TwoPI*u2) * u1p, sin(TwoPI*u2) * u1p, u1);
		}

		vec3 getSamplingVector(){
			vec2 st = gl_GlobalInvocationID.xy/vec2(imageSize(outputTexture));
			vec2 uv = 2.0 * vec2(st.x, 1.0-st.y) - vec2(1.0);

			vec3 ret;

			if(gl_GlobalInvocationID.z == 0)      ret = vec3(1.0,  uv.y, -uv.x);
			else if(gl_GlobalInvocationID.z == 1) ret = vec3(-1.0, uv.y,  uv.x);
			else if(gl_GlobalInvocationID.z == 2) ret = vec3(uv.x, 1.0, -uv.y);
			else if(gl_GlobalInvocationID.z == 3) ret = vec3(uv.x, -1.0, uv.y);
			else if(gl_GlobalInvocationID.z == 4) ret = vec3(uv.x, uv.y, 1.0);
			else if(gl_GlobalInvocationID.z == 5) ret = vec3(-uv.x, uv.y, -1.0);
			return normalize(ret);
		}

		void computeBasisVectors(const vec3 N, out vec3 S, out vec3 T){
			T = cross(N, vec3(0.0, 1.0, 0.0));
			T = mix(cross(N, vec3(1.0, 0.0, 0.0)), T, step(Epsilon, dot(T, T)));

			T = normalize(T);
			S = normalize(cross(N, T));
		}

		vec3 tangentToWorld(const vec3 v, const vec3 N, const vec3 S, const vec3 T){
			return S * v.x + T * v.y + N * v.z;
		}

		void main(void){
			vec3 N = getSamplingVector();
	
			vec3 S, T;
			computeBasisVectors(N, S, T);

			vec3 irradiance = vec3(0);
			for(uint i=0; i<NumSamples; ++i) {
				vec2 u  = sampleHammersley(i);
				vec3 Li = tangentToWorld(sampleHemisphere(u.x, u.y), N, S, T);
				float cosTheta = max(0.0, dot(Li, N));

				irradiance += 2.0 * textureLod(inputTexture, Li, 0).rgb * cosTheta;
			}
			irradiance /= vec3(NumSamples);

			imageStore(outputTexture, ivec3(gl_GlobalInvocationID), vec4(irradiance, 1.0));
		}
	)");
	mDiffuseIrradiancePipeline->setShaderStage({ QRhiShaderStage::Compute,compute });
	mDiffuseIrradiancePipeline->create();

	mBrdfLutPipeline.reset(mRhi->newComputePipeline());
	mBrdfLutBindings.reset(mRhi->newShaderResourceBindings());
	mBrdfLutBindings->setBindings({
		QRhiShaderResourceBinding::imageStore(0, QRhiShaderResourceBinding::ComputeStage,mBrdfLut.get(),0),
	});
	mBrdfLutBindings->create();
	mBrdfLutPipeline->setShaderResourceBindings(mBrdfLutBindings.get());
	compute = mRhi->newShaderFromCode(QShader::ComputeStage, R"(#version 450
		const float PI = 3.141592;
		const float TwoPI = 2 * PI;
		const float Epsilon = 0.001; // This program needs larger eps.

		const uint NumSamples = 1024;
		const float InvNumSamples = 1.0 / float(NumSamples);

		layout(local_size_x=32, local_size_y=32, local_size_z=1) in;
		layout(binding=0, rgba32f) restrict writeonly uniform image2D LUT;

		float radicalInverse_VdC(uint bits){
			bits = (bits << 16u) | (bits >> 16u);
			bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
			bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
			bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
			bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
			return float(bits) * 2.3283064365386963e-10; // / 0x100000000
		}

		vec2 sampleHammersley(uint i){
			return vec2(i * InvNumSamples, radicalInverse_VdC(i));
		}

		vec3 sampleGGX(float u1, float u2, float roughness){
			float alpha = roughness * roughness;

			float cosTheta = sqrt((1.0 - u2) / (1.0 + (alpha*alpha - 1.0) * u2));
			float sinTheta = sqrt(1.0 - cosTheta*cosTheta); // Trig. identity
			float phi = TwoPI * u1;

			return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
		}

		float gaSchlickG1(float cosTheta, float k){
			return cosTheta / (cosTheta * (1.0 - k) + k);
		}

		float gaSchlickGGX_IBL(float cosLi, float cosLo, float roughness){
			float r = roughness;
			float k = (r * r) / 2.0; // Epic suggests using this roughness remapping for IBL lighting.
			return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
		}

		void main(void){
			float cosLo = gl_GlobalInvocationID.x / float(imageSize(LUT).x);
			float roughness = gl_GlobalInvocationID.y / float(imageSize(LUT).y);

			cosLo = max(cosLo, Epsilon);

			vec3 Lo = vec3(sqrt(1.0 - cosLo*cosLo), 0.0, cosLo);

			float DFG1 = 0;
			float DFG2 = 0;

			for(uint i=0; i<NumSamples; ++i) {
				vec2 u  = sampleHammersley(i);

				vec3 Lh = sampleGGX(u.x, u.y, roughness);

				vec3 Li = 2.0 * dot(Lo, Lh) * Lh - Lo;

				float cosLi   = Li.z;
				float cosLh   = Lh.z;
				float cosLoLh = max(dot(Lo, Lh), 0.0);

				if(cosLi > 0.0) {
					float G  = gaSchlickGGX_IBL(cosLi, cosLo, roughness);
					float Gv = G * cosLoLh / (cosLh * cosLo);
					float Fc = pow(1.0 - cosLoLh, 5);

					DFG1 += (1 - Fc) * Gv;
					DFG2 += Fc * Gv;
				}
			}

			imageStore(LUT, ivec2(gl_GlobalInvocationID), vec4(DFG1, DFG2, 0, 1) * InvNumSamples);
		}

	)");
	mBrdfLutPipeline->setShaderStage({ QRhiShaderStage::Compute,compute });
	mBrdfLutPipeline->create();
}

void QSkyRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	if (sigUploadEquirectTexture.receive()) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->uploadTexture(mEquirectTexture.get(), mSkyBoxImage);
		if (!bIsEquirectangular) {
			std::array<QImage, 6> subImages = AssetUtils::resolveCubeSubImages(mSkyBoxImage);
			QRhiTextureSubresourceUploadDescription subresDesc[6];
			for (int i = 0; i < 6; i++) {
				subresDesc[i].setImage(subImages[i].scaled(QSize(kEnvMapSize, kEnvMapSize)));
			}
			QRhiTextureUploadDescription desc = QRhiTextureUploadDescription({
				{ 0, 0, subresDesc[0] },  // +X
				{ 1, 0, subresDesc[1] },  // -X
				{ 2, 0, subresDesc[2] },  // +Y
				{ 3, 0, subresDesc[3] },  // -Y
				{ 4, 0, subresDesc[4] },  // +Z
				{ 5, 0, subresDesc[5] }   // -Z
			});
			batch->uploadTexture(mSkyCube.get(), desc);
			batch->generateMips(mSkyCube.get());
			sigComputeIBLCube.request();
		}
		cmdBuffer->resourceUpdate(batch);
	}
	if (sigComputeSkyCube.receive()) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->uploadTexture(mEquirectTexture.get(), mSkyBoxImage);
		cmdBuffer->beginComputePass(batch);
		cmdBuffer->setComputePipeline(mSkyCubePipeline.get());
		cmdBuffer->setShaderResources();
		cmdBuffer->dispatch(kEnvMapSize / 32, kEnvMapSize / 32, 6);
		cmdBuffer->endComputePass();
		batch = mRhi->nextResourceUpdateBatch();
		batch->generateMips(mSkyCube.get());
		cmdBuffer->resourceUpdate(batch);
		sigComputeIBLCube.request();
	}
	if (sigComputeIBLCube.receive()) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		QRhiTextureCopyDescription desc;
		for (int i = 0; i < 6; i++) {
			desc.setDestinationLayer(i);
			desc.setSourceLayer(i);
			batch->copyTexture(mPrefilteredSpecularCube.get(), mSkyCube.get(), desc);
		}
		cmdBuffer->resourceUpdate(batch);

		int levels = mRhi->mipLevelsForSize(mSkyCube->pixelSize());
		const float deltaRoughness = 1.0f / qMax(float(levels - 1), 1.0f);
		char* ptr = mPrefilteredSpecularCubeUniformBuffer->beginFullDynamicBufferUpdateForCurrentFrame();
		for (int i = 1; i < levels; i++) {
			*(float*)ptr = i * deltaRoughness;
			ptr += mRhi->ubufAlignment();
		}
		mPrefilteredSpecularCubeUniformBuffer->endFullDynamicBufferUpdateForCurrentFrame();

		for (int level = 1, size = kEnvMapSize / 2; level < levels; ++level, size /= 2) {
			const int numGroups = qMax(1, size / 32);
			mPrefilteredSpecularCubeBindings->setBindings({
				QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::ComputeStage, mSkyCube.get(),mSampler.get()),
				QRhiShaderResourceBinding::imageStore(1, QRhiShaderResourceBinding::ComputeStage,mPrefilteredSpecularCube.get(),level),
				QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::ComputeStage,mPrefilteredSpecularCubeUniformBuffer.get(), mRhi->ubufAlignment() * (level-1),sizeof(float)),
			});
			mPrefilteredSpecularCubeBindings->create();
			cmdBuffer->beginComputePass();
			cmdBuffer->setComputePipeline(mPrefilteredSpecularCubePipeline.get());
			cmdBuffer->setShaderResources(mPrefilteredSpecularCubeBindings.get());
			cmdBuffer->dispatch(numGroups, numGroups, 6);
			cmdBuffer->endComputePass();

		}

		cmdBuffer->beginComputePass();
		cmdBuffer->setComputePipeline(mDiffuseIrradiancePipeline.get());
		cmdBuffer->setShaderResources();
		cmdBuffer->dispatch(mDiffuseIrradianceCube->pixelSize().width() / 32, mDiffuseIrradianceCube->pixelSize().height() / 32, 6);
		cmdBuffer->endComputePass();

		cmdBuffer->beginComputePass();
		cmdBuffer->setComputePipeline(mBrdfLutPipeline.get());
		cmdBuffer->setShaderResources();
		cmdBuffer->dispatch(mBrdfLut->pixelSize().width() / 32, mBrdfLut->pixelSize().height() / 32, 6);
		cmdBuffer->endComputePass();
	}
}

