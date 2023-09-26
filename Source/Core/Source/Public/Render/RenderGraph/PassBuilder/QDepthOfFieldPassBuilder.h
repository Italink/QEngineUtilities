#ifndef QDepthOfFieldPassBuilder_h__
#define QDepthOfFieldPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QDepthOfFieldPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QDepthOfFieldPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(QRhiTextureRef, PositionTexture);
		QRP_INPUT_ATTR(float, Focus);
		QRP_INPUT_ATTR(float, FocalLength);
		QRP_INPUT_ATTR(float, Aperture);
		QRP_INPUT_ATTR(int, ApertureBlades);
		QRP_INPUT_ATTR(float, BokehSqueeze);
		QRP_INPUT_ATTR(float, BokehSqueezeFalloff);
		QRP_INPUT_ATTR(int, Iterations);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QDepthOfFieldPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, DepthOfFieldResult);
	QRP_OUTPUT_END()
public:
	QDepthOfFieldPassBuilder();

protected:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	IRenderer* mRenderer = nullptr;
	QShader mDepthOfFieldFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock {
		QGenericMatrix<4, 4, float> VP;
		float focus = 0.05f;
		float focalLength = 20.0f;
		float aperture = 2.5f;
		int apertureBlades = 5;
		float bokehSqueeze = 0.0f;
		float bokehSqueezeFalloff = 1.0f;
		float aspectRatio = 1.77f;
		int iterations = 64;
	}mParams;
	RTResource mRT;
	QRhiBufferRef mUniformBuffer;
	QRhiSamplerRef mSampler;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QDepthOfFieldPassBuilder_h__
