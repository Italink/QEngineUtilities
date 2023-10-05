#ifndef QMotionBlurPassBuilder_h__
#define QMotionBlurPassBuilder_h__

#include "IRenderPassBuilder.h"

class QENGINECORE_API QMotionBlurPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QMotionBlurPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(QRhiTextureRef, PositionTexture);
		QRP_INPUT_ATTR(int, MotionBlurSize);
		QRP_INPUT_ATTR(float, MotionBlurSeparation);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QMotionBlurPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, MotionBlurReslut);
	QRP_OUTPUT_END()
public:
	QMotionBlurPassBuilder();

public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mMontionBlurFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock {
		QGenericMatrix<4, 4, float> previousViewToWorld;
		QGenericMatrix<4, 4, float> worldToView;
		QGenericMatrix<4, 4, float> projection;
		uint32_t size = 0;
		float separation;
	}mParams;
	RTResource mRT;
	QRhiBufferRef mUniformBuffer;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QMotionBlurPassBuilder_h__
