//#include "Render/Pass/QToneMappingRenderPass.h"
//
//QToneMappingRenderPass::QToneMappingRenderPass(){}
//
//void QToneMappingRenderPass::setGamma(float val) {
//	mParams.gamma = val;
//	sigUpdateParams.request();
//}
//
//void QToneMappingRenderPass::setExposure(float val) {
//	mParams.exposure = val;
//	sigUpdateParams.request();
//}
//
//void QToneMappingRenderPass::setPureWhite(float val) {
//	mParams.pureWhite = val;
//	sigUpdateParams.request();
//}
//
//void QToneMappingRenderPass::resizeAndLinkNode(const QSize& size) {
//	QRhiTexture* src = getTextureIn_Src();
//	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA8, src->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
//	mRT.colorAttachment->create();
//	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
//	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
//	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
//	mRT.renderTarget->create();
//
//	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
//		QRhiSampler::Linear,
//		QRhiSampler::None,
//		QRhiSampler::ClampToEdge,
//		QRhiSampler::ClampToEdge)
//	);
//	mSampler->create();
//
//	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
//	mUniformBuffer->create();
//
//	mBindings.reset(mRhi->newShaderResourceBindings());
//	mBindings->setBindings({
//		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,src, mSampler.get()),
//		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
//	});
//	mBindings->create();
//	registerTextureOut_Result(mRT.colorAttachment.get());
//	sigUpdateParams.request();
//}
//
//void QToneMappingRenderPass::compile() {
//	mPipeline.reset(mRhi->newGraphicsPipeline());
//	QRhiGraphicsPipeline::TargetBlend blendState;
//	blendState.enable = true;
//	mPipeline->setTargetBlends({ blendState });
//	mPipeline->setSampleCount(mRT.renderTarget->sampleCount());
//
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
//	QShader vs = QRhiHelper::newShaderFromCode( QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());
//
//	QShader fs = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
//		layout (location = 0) in vec2 vUV;
//		layout (location = 0) out vec4 outFragColor;
//		layout (binding = 0 ) uniform sampler2D uSrcTexture;
//		layout (binding = 1 ) uniform Params{
//			float gamma;
//			float exposure;
//			float pureWhite;
//		}params;
//		void main() {
//			vec3 color = texture(uSrcTexture, vUV).rgb * params.exposure;
//
//			// Reinhard tone mapping operator.
//			// see: "Photographic Tone Reproduction for Digital Images", eq. 4
//			float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
//			float mappedLuminance = (luminance * (1.0 + luminance/(params.pureWhite*params.pureWhite))) / (1.0 + luminance);
//
//			// Scale color by ratio of average luminances.
//			vec3 mappedColor = (mappedLuminance / luminance) * color;
//
//			// Gamma correction.
//			outFragColor = vec4(pow(mappedColor, vec3(1.0/params.gamma)), 1.0);
//		}
//	)");
//
//	mPipeline->setShaderStages({
//		{ QRhiShaderStage::Vertex, vs },
//		{ QRhiShaderStage::Fragment, fs }
//	});
//
//	QRhiVertexInputLayout inputLayout;
//
//	mPipeline->setVertexInputLayout(inputLayout);
//	mPipeline->setShaderResourceBindings(mBindings.get());
//	mPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
//	mPipeline->create();
//}
//
//void QToneMappingRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
//	if (sigUpdateParams.ensure()) {
//		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
//		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(Params), &mParams);
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
