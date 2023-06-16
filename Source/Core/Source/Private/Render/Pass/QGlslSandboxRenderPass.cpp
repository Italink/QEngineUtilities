#include "Render/Pass/QGlslSandboxRenderPass.h"
#include <QRegularExpression>
#include "QDateTime"

void QGlslSandboxRenderPass::setShaderCode(QString val) {
	mShaderCode = mRawShaderCode = val;
	mShaderCode.remove(QRegularExpression("uniform +float +time *;"));
	mShaderCode.remove(QRegularExpression("uniform +vec2 +resolution *;"));
	mShaderCode.remove(QRegularExpression("uniform +vec2 +mouse; *"));
	sigRecompile.request();
}

void QGlslSandboxRenderPass::resizeAndLinkNode(const QSize& size) {
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_Output(mRT.colorAttachment.get());
	sigRecompile.ensure();
}

void QGlslSandboxRenderPass::compile() {
	mUniformBlock.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock)));
	mUniformBlock->create();

	mPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	mPipeline->setTargetBlends({ blendState });
	mPipeline->setSampleCount(mRT.renderTarget->sampleCount());

	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 450
		layout (location = 0) out vec2 sufacePosition;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			gl_Position = vec4(vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2)* 2 -1, 0.0f, 1.0f);
			sufacePosition = gl_Position.xy;
			#if Y_UP_IN_NDC
				sufacePosition.y = - sufacePosition.y;
			#endif 
		})"
		, QShaderDefinitions()
		.addDefinition("Y_UP_IN_NDC", mRhi->isYUpInNDC())
	);

	QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 sufacePosition;
		layout (binding = 0) uniform UniformBlock{
			vec2 mouse;
			vec2 resolution;
			vec2 sufaceSize;
			float time;
		}UBO;
		layout (location = 0) out vec4 fragColor;

		#define mouse UBO.mouse
		#define resolution UBO.resolution
		#define sufaceSize UBO.sufaceSize
		#define time UBO.time

		#define iTime  time
        #define iMouse mouse
        #define iResolution resolution

		#define gl_FragColor fragColor

	)" + mShaderCode.toLocal8Bit());
	if (!fs.isValid()) {
		mPipeline.reset();
		return;
	}

	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	QRhiVertexInputLayout inputLayout;
	mPipeline->setVertexInputLayout(inputLayout);

	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::FragmentStage,mUniformBlock.get())
	});
	mBindings->create();
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(mRT.renderTarget->renderPassDescriptor());
	mPipeline->create();
}

void QGlslSandboxRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	if (sigRecompile.ensure()) {
		compile();
	}
	if (mPipeline) {
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		UniformBlock ub;
		ub.time = QTime::currentTime().msecsSinceStartOfDay() / 1000.0f;
		ub.resolution = QVector2D(mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height());
		ub.mouse = QVector2D((QCursor::pos().x() / (float)ub.resolution.x()), (ub.resolution.y() - QCursor::pos().y()) / (float)ub.resolution.y());

		batch->updateDynamicBuffer(mUniformBlock.get(), 0, sizeof(UniformBlock), &ub);
		cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mBindings.get());
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
}