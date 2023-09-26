#ifndef QSsaoPassBuilder_h__
#define QSsaoPassBuilder_h__

#include "IRenderPassBuilder.h"

class QENGINECORE_API QSsaoPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QSsaoPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, PositionTexture);
		QRP_INPUT_ATTR(QRhiTextureRef, NormalTexture);
		QRP_INPUT_ATTR(int, SampleSize);
		QRP_INPUT_ATTR(float, Radius);
		QRP_INPUT_ATTR(float, Bias);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QSsaoPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, SsaoResult);
	QRP_OUTPUT_END()
public:
	QSsaoPassBuilder();
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	void refreshSsaoState();
private:
	IRenderer* mRenderer = nullptr;
	QShader mSsaoFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock {
		QGenericMatrix<4, 4, float> VP;
		float radius = 2.0f;
		float bias = 0.1f;
		uint32_t size = 0;
		uint32_t padding;
		QVector4D samples[128];
		QVector4D noise[16];
	}mSsaoState;
	RTResource mRT;
	QRhiBufferRef mUniformBuffer;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QSsaoPassBuilder_h__