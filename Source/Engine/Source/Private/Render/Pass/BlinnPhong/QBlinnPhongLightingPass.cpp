//#include "Render/Pass/BlinnPhong/QBlinnPhongLightingPass.h"
//
//QColor4D QBlinnPhongLightingPass::getAmbientLight() const {
//	return QVector4D(mParams.ambientLight, 1.0f);
//}
//
//void QBlinnPhongLightingPass::setAmbientLight(QColor4D val) {
//	mParams.ambientLight = val.toVector3D();
//	sigUpdateParams.request();
//}
//
//void QBlinnPhongLightingPass::resizeAndLinkNode(const QSize& size) {
//	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
//	mRT.colorAttachment->create();
//	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
//	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
//	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
//	mRT.renderTarget->create();
//	registerTextureOut_FragColor(mRT.colorAttachment.get());
//
//	mUniformBlock.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
//	mUniformBlock->create();
//
//	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
//		QRhiSampler::Linear,
//		QRhiSampler::Linear,
//		QRhiSampler::ClampToEdge,
//		QRhiSampler::ClampToEdge));
//	mSampler->create();
//
//	mBindings.reset(mRhi->newShaderResourceBindings());
//	mBindings->setBindings({
//		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Albedo(), mSampler.get()),
//		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Position(), mSampler.get()),
//		QRhiShaderResourceBinding::sampledTexture(2,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Normal(), mSampler.get()),
//		QRhiShaderResourceBinding::sampledTexture(3,QRhiShaderResourceBinding::FragmentStage, getTextureIn_Specular(), mSampler.get()),
//		QRhiShaderResourceBinding::uniformBuffer(4,QRhiShaderResourceBinding::FragmentStage, mUniformBlock.get())
//	});
//	mBindings->create();
//
//	sigUpdateParams.request();
//}
//
//void QBlinnPhongLightingPass::compile() {
//	mPipeline.reset(mRhi->newGraphicsPipeline());
//	QRhiGraphicsPipeline::TargetBlend blendState;
//	blendState.enable = false;
//	blendState.srcColor = QRhiGraphicsPipeline::SrcAlpha;
//	blendState.enable = true;
//	mPipeline->setTargetBlends({ blendState });
//	mPipeline->setSampleCount(mRT.renderTarget->sampleCount());
//	QString vsCode = R"(#version 450
//		layout (location = 0) out vec2 vUV;
//		out gl_PerVertex{
//			vec4 gl_Position;
//		};
//		void main() {
//			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
//			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
//			%1
//		}
//	)";
//	QShader vs = QRhiHelper::newShaderFromCode(mRhi, QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());
//	QShader fs = QRhiHelper::newShaderFromCode(mRhi, QShader::FragmentStage, R"(#version 450
//		layout(binding = 0) uniform sampler2D albedoTexture;
//		layout(binding = 1) uniform sampler2D positionTexture;
//		layout(binding = 2) uniform sampler2D normalTexture;
//		layout(binding = 3) uniform sampler2D specularTexture;
//		struct PointLightParams {
//			vec3 radiance;
//			float distance;
//		};
//		struct DirectionLightParams {
//			vec3 radiance;
//			vec3 direction;
//		};
//		layout(binding = 4) uniform Params{
//			vec3 ambientLight;
//			int numOfPointLight;
//			PointLightParams pointLights[64];
//
//			int numOfdirectionLight;
//			DirectionLightParams directionLights[64];
//		}params;
//
//		layout(location = 0) in vec2 vUV;
//		layout(location = 0) out vec4 outFragColor;
//
//		void main(){
//			vec4 albedo = texture(albedoTexture, vUV);
//			vec3 position = texture(positionTexture, vUV).rgb;
//			float specular = texture(specularTexture, vUV).r;
//			vec3 normal = texture(normalTexture, vUV).rgb;
//			vec3 directLighting = params.ambientLight * albedo.rgb;
//			outFragColor = vec4(params.ambientLight * albedo.rgb , albedo.a);
//		}
//	)");
//
//	mPipeline->setShaderStages({
//		{ QRhiShaderStage::Vertex, vs },
//		{ QRhiShaderStage::Fragment, fs }
//	});
//	QRhiVertexInputLayout inputLayout;
//	mPipeline->setVertexInputLayout(inputLayout);
//	mPipeline->setShaderResourceBindings(mBindings.get());
//	mPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
//	mPipeline->create();
//}
//
//void QBlinnPhongLightingPass::render(QRhiCommandBuffer* cmdBuffer) {
//	if (sigUpdateParams.ensure()) {
//		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
//		batch->updateDynamicBuffer(mUniformBlock.get(), 0, sizeof(Params), &mParams);
//		cmdBuffer->resourceUpdate(batch);
//	}
//	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
//	cmdBuffer->setGraphicsPipeline(mPipeline.get());
//	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
//	cmdBuffer->setShaderResources(mBindings.get());
//	cmdBuffer->draw(4);
//	cmdBuffer->endPass();
//}
//
