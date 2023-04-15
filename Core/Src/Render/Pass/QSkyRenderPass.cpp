#include "QSkyRenderPass.h"
#include "Asset/AssetUtils.h"

static constexpr int kEnvMapSize = 1024;


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
		image = QImage(inPath).convertToFormat(QImage::Format::Format_RGBA32FPx4);
		bIsEquirectangular = false;
	}
	setSkyBoxImage(image);
}

void QSkyRenderPass::setSkyBoxImage(QImage inImage) {
	if (!inImage.isNull()) {
		mSkyBoxImage = inImage;
		sigUploadEquirectTexture.request();
	}
}

QImage QSkyRenderPass::getSkyBoxImage() const {
	return mSkyBoxImage;
}

void QSkyRenderPass::resizeAndLinkNode(const QSize& size) {
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_SkyTexture(mRT.colorAttachment.get());
}

void QSkyRenderPass::compile() {
	if (!sigInitResource.ensure()) {
		return;
	}
	mSkyCube.reset(mRhi->newTexture(QRhiTexture::RGBA32F,  QSize(kEnvMapSize, kEnvMapSize), 1, QRhiTexture::CubeMap | QRhiTexture::MipMapped | QRhiTexture::UsedWithGenerateMips | QRhiTexture::UsedWithLoadStore));
	mSkyCube->create();
	registerTextureOut_SkyCube(mSkyCube.get());
	
	mEquirectTexture.reset(mRhi->newTexture(QRhiTexture::RGBA32F, mSkyBoxImage.size(), 1));
	mEquirectTexture->create();
	registerTextureOut_Equirect(mEquirectTexture.get());
	sigUploadEquirectTexture.request();

	mSkyboxUniformBlock.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(SkyboxUniformBlock)));
	mSkyboxUniformBlock->create();

	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::Repeat,
		QRhiSampler::Repeat));
	mSampler->create();

	mSkyboxBindings.reset(mRhi->newShaderResourceBindings());
	mSkyboxBindings->setBindings({
		QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage, mSkyboxUniformBlock.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, mSkyCube.get(), mSampler.get()),
	});
	mSkyboxBindings->create();

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

	sigUploadSkyboxVertics.request();
}

void QSkyRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	if (sigUploadSkyboxVertics.ensure()) {
		batch->uploadStaticBuffer(mSkyboxVertexBuffer.get(), CubeData);
	}
	SkyboxUniformBlock skyUb;
	QMatrix4x4 MVP = mRenderer->getCamera()->getMatrixClipWithCorr(mRhi) * mRenderer->getCamera()->getMatrixView();
	MVP.scale(2000);
	skyUb.MVP = MVP.toGenericMatrix<4, 4>();
	batch->updateDynamicBuffer(mSkyboxUniformBlock.get(), 0, sizeof(SkyboxUniformBlock), &skyUb);
	
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	cmdBuffer->setGraphicsPipeline(mSkyboxPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mSkyboxBindings.get());
	const QRhiCommandBuffer::VertexInput VertexInput(mSkyboxVertexBuffer.get(), 0);
	cmdBuffer->setVertexInput(0, 1, &VertexInput);
	cmdBuffer->draw(36);
	cmdBuffer->endPass();

	if (sigUploadEquirectTexture.ensure()) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->uploadTexture(mEquirectTexture.get(), mSkyBoxImage);
		if (!bIsEquirectangular) {
			std::array<QImage, 6> subImages = AssetUtils::resolveCubeSubImages(mSkyBoxImage);
			QRhiTextureSubresourceUploadDescription subresDesc[6];
			for (int i = 0; i < 6; i++) {
				QImage subImage = subImages[i].scaled(QSize(kEnvMapSize, kEnvMapSize)).convertToFormat(QImage::Format::Format_RGBA32FPx4);
				subresDesc[i].setImage(subImage);
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
		}
		cmdBuffer->resourceUpdate(batch);
	}
	if (sigComputeSkyCube.ensure()) {
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
	}
}