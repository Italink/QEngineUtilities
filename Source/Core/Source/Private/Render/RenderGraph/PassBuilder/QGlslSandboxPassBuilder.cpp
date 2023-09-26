#include "QGlslSandboxPassBuilder.h"
#include <QRegularExpression>
#include "QDateTime"

void QGlslSandboxPassBuilder::setup(QRenderGraphBuilder& builder)
{
	if (mInput._ShaderCode != mShaderCode) {
		mShaderCode = mInput._ShaderCode;
		QString currShaderCode = mShaderCode;
		currShaderCode.remove(QRegularExpression("uniform +float +time *;"));
		currShaderCode.remove(QRegularExpression("uniform +vec2 +resolution *;"));
		currShaderCode.remove(QRegularExpression("uniform +vec2 +mouse; *"));
		mGlslSandboxFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
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

		)" + currShaderCode.toLocal8Bit());
	}

	if (!mGlslSandboxFS.isValid())
		return;

	builder.setupTexture(mRT.colorAttachment, "GlslTexture", QRhiTexture::RGBA32F, builder.mainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "GlslRenderTarget", { mRT.colorAttachment.get() });

	builder.setupBuffer(mUniformBuffer, "GlslUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformBlock));

	builder.setupShaderResourceBindings(mBindings, "GlslBindings", {
		QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
	});

	QRhiGraphicsPipelineState PSO;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, builder.fullScreenVS() },
		{ QRhiShaderStage::Fragment, mGlslSandboxFS }
	};
	builder.setupGraphicsPipeline(mPipeline, "GlslPipeline", PSO);

	mOutput.GlslSandboxResult = mRT.colorAttachment;
}

void QGlslSandboxPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	if (!mGlslSandboxFS.isValid())
		return;
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	UniformBlock ub;
	ub.time = QTime::currentTime().msecsSinceStartOfDay() / 1000.0f;
	ub.resolution = QVector2D(mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height());
	ub.mouse = QVector2D((QCursor::pos().x() / (float)ub.resolution.x()), (ub.resolution.y() - QCursor::pos().y()) / (float)ub.resolution.y());
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &ub);
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
