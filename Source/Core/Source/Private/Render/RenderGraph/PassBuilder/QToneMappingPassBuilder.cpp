#include "QToneMappingPassBuilder.h"

QToneMappingPassBuilder::QToneMappingPassBuilder()
{
	mToneMappingFS = QRhiHelper::newShaderFromCode( QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		layout (binding = 0 ) uniform sampler2D uBaseColorTexture;
		layout (binding = 1 ) uniform Params{
			float gamma;
			float exposure;
			float pureWhite;
		}params;
		void main() {
			vec3 color = texture(uBaseColorTexture, vUV).rgb * params.exposure;

			// Reinhard tone mapping operator.
			// see: "Photographic Tone Reproduction for Digital Images", eq. 4
			float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
			float mappedLuminance = (luminance * (1.0 + luminance/(params.pureWhite*params.pureWhite))) / (1.0 + luminance);

			// Scale color by ratio of average luminances.
			vec3 mappedColor = (mappedLuminance / luminance) * color;

			// Gamma correction.
			outFragColor = vec4(pow(mappedColor, vec3(1.0/params.gamma)), 1.0);
		}
	)");
}

void QToneMappingPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	UniformBlock params;
	params.gamma = mInput._Gamma;
	params.pureWhite = mInput._PureWhite;
	params.exposure = mInput._Exposure;
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &params);
	cmdBuffer->resourceUpdate(batch);
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mBindings.get());
	cmdBuffer->draw(4);
	cmdBuffer->endPass();
}
