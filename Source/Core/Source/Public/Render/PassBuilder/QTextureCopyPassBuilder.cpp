#include "QTextureCopyPassBuilder.h"
#include "QRhiHelper.h"

QTexutreCopyPassBuilder::QTexutreCopyPassBuilder()
{
	mVertexShader = QRhiHelper::newShaderFromCode(QShader::VertexStage, R"(#version 440
		layout (location = 0) out vec2 vUV;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
		}
	)");
	Q_ASSERT(mVertexShader.isValid());

	mFragmentShader = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
		layout (binding = 0) uniform sampler2D uSamplerColor;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		void main() {
			outFragColor = vec4(texture(uSamplerColor, vUV).rgb,1.0f);
		}
	)");
	Q_ASSERT(mFragmentShader.isValid());
}

void QTexutreCopyPassBuilder::setup(QRGBuilder& builder)
{
	builder.setupSampler(mSampler,"CopyPass.Samper", QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None, QRhiSampler::Repeat, QRhiSampler::Repeat, QRhiSampler::Repeat);
	builder.setupShaderResourceBindings(mShaderBindings, "CopyPass.Bindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,Input.SrcTexture.get() ,mSampler.get())
	});
	QRhiGraphicsPipelineState PSO;
	PSO.shaderResourceBindings = mShaderBindings.get();														//绑定到流水线
	PSO.sampleCount = Input.DstRenderTarget->sampleCount();
	PSO.renderPassDesc = Input.DstRenderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		QRhiShaderStage(QRhiShaderStage::Vertex, mVertexShader),
		QRhiShaderStage(QRhiShaderStage::Fragment, mFragmentShader)
	};
	builder.setupGraphicsPipeline(mPipeline, "CopyPass.Pipeline", PSO);
}

void QTexutreCopyPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	if (Input.SrcTexture) {
		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cmdBuffer->beginPass(Input.DstRenderTarget, clearColor, dsClearValue, nullptr);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, Input.DstRenderTarget->pixelSize().width(), Input.DstRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mShaderBindings.get());
		cmdBuffer->draw(4);

		cmdBuffer->endPass();
	}													
}
