#include "Render/Pass/QMotionBlurRenderPass.h"

QMotionBlurRenderPass::QMotionBlurRenderPass() {
	setMotionBlurSize(2);
	setMotionBlurSeparation(1.0f);
}

void QMotionBlurRenderPass::setMotionBlurSize(int size) {
	if (size <= 0 || size == mParams.size)
		return;
	mParams.size = size;
	sigUpdateParams.request();
}

void QMotionBlurRenderPass::setMotionBlurSeparation(float val)
{
	mParams.separation = val;
	sigUpdateParams.request();
}


void QMotionBlurRenderPass::resizeAndLinkNode(const QSize& size) {
	auto baseuBaseColor = getTextureIn_BaseColor();
	auto uPosition = getTextureIn_Position();
	mMotionBlurRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, baseuBaseColor->pixelSize() , 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mMotionBlurRT.colorAttachment->create();
	mMotionBlurRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mMotionBlurRT.colorAttachment.get() }));
	
	renderPassDesc.reset(mMotionBlurRT.renderTarget->newCompatibleRenderPassDescriptor());

	mMotionBlurRT.renderTarget->setRenderPassDescriptor(renderPassDesc.get());
	mMotionBlurRT.renderTarget->create();
	
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge)
	);
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(Params)));
	mUniformBuffer->create();

	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage, baseuBaseColor, mSampler.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage, uPosition, mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(2,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get()),
	});
	mBindings->create();

	registerTextureOut_Result(mMotionBlurRT.colorAttachment.get());

	sigUpdateParams.request();
}

void QMotionBlurRenderPass::compile() {
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
	mPipeline->setTargetBlends({ blendState });
	mPipeline->setSampleCount(mMotionBlurRT.renderTarget->sampleCount());

	QShader vs = QRhiHelper::newShaderFromCode(mRhi, QShader::VertexStage, R"(#version 450
		layout (location = 0) out vec2 vUV;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
#if Y_UP_IN_NDC
			gl_Position.y = - gl_Position.y;
#endif 
		})"
		, QShaderDefinitions()
		.addDefinition("Y_UP_IN_NDC", mRhi->isYUpInNDC())
	);

	QShader fs = QRhiHelper::newShaderFromCode(mRhi, QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		
		layout (binding = 0) uniform sampler2D uBaseColor;
		layout (binding = 1) uniform sampler2D uPosition;
		layout (binding = 2) uniform MontionBlurParams{
			mat4 previousViewToWorld;
			mat4 worldToView;
			mat4 projection;
			int size;
			float separation;
		}params;
		
		void main(){
			  vec2 texSize  = textureSize(uBaseColor, 0).xy;
			  vec2 vUV = gl_FragCoord.xy / texSize;

			  outFragColor = texture(uBaseColor,    vUV);

			  vec4 position1 = texture(uPosition, vUV);

			  if (params.size <= 0 || params.separation <= 0.0) { return; }

			  vec4 position0 =  params.worldToView * params.previousViewToWorld * position1;

			  position0      = params.projection * position0;
			  position0.xyz /= position0.w;
			  position0.xy   = position0.xy * 0.5 + 0.5;

			  position1      = params.projection * position1;
			  position1.xyz /= position1.w;
			  position1.xy   = position1.xy * 0.5 + 0.5;

			  vec2 direction = position1.xy - position0.xy;

			  direction.xy *= params.separation;

			  vec2 forward  = vUV;
			  vec2 backward = vUV;

			  float count = 1.0;

			  for (int i = 0; i < params.size; ++i) {
				forward  += direction;
				backward -= direction;

				outFragColor +=
				  texture
					( uBaseColor
					, forward
					);
				outFragColor +=
				  texture
					( uBaseColor
					, backward
					);

				count += 2.0;
			  }
			  outFragColor /= count;
		}
	)");
	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	QRhiVertexInputLayout inputLayout;
	mPipeline->setVertexInputLayout(inputLayout);
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(mMotionBlurRT.renderTarget->renderPassDescriptor());
	mPipeline->create();

	QMatrix4x4 view = mRenderer->getCamera()->getViewMatrix();
	mParams.worldToView = view.toGenericMatrix<4, 4>();
	mParams.previousViewToWorld = view.inverted().toGenericMatrix<4, 4>();
}

void QMotionBlurRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	mParams.projection = mRenderer->getCamera()->getProjectionMatrixWithCorr(mRhi).toGenericMatrix<4, 4>();
	QMatrix4x4 view = mRenderer->getCamera()->getViewMatrix();
	mParams.worldToView = view.toGenericMatrix<4, 4>();

	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(Params), &mParams);

	cmdBuffer->resourceUpdate(batch);
	cmdBuffer->beginPass(mMotionBlurRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mMotionBlurRT.renderTarget->pixelSize().width(), mMotionBlurRT.renderTarget->pixelSize().height()));
	cmdBuffer->draw(4);
	cmdBuffer->endPass();

	mParams.previousViewToWorld = view.inverted().toGenericMatrix<4, 4>();
}
