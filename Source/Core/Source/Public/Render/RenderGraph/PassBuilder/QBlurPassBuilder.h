#ifndef QBlurRenderPass_h__
#define QBlurRenderPass_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QBlurPassBuilder : public IRenderPassBuilder {
public:
	QBlurPassBuilder();

	QRP_INPUT_BEGIN(QBlurPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(int, BlurIterations) = 2;
		QRP_INPUT_ATTR(int, BlurSize) = 20;
		QRP_INPUT_ATTR(int, DownSampleCount) = 1;
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QBlurPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, BlurResult);
	QRP_OUTPUT_END()
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	void refreshKernel();
private:
	QShader mDownSampleFS;
	QShader mBlurHFS;
	QShader mBlurVFS;

	QRhiSamplerRef mSampler;
	QRhiBufferRef mUniformBuffer;
	struct BlurRT {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	BlurRT mBlurRT[2];
	QRhiGraphicsPipelineRef mPipelineDownSample;
	QRhiGraphicsPipelineRef mPipelineH;
	QRhiGraphicsPipelineRef mPipelineV;

	QRhiShaderResourceBindingsRef mBindingsDownSample;
	QRhiShaderResourceBindingsRef mBindingsH;
	QRhiShaderResourceBindingsRef mBindingsV;

	struct BlurKernel {
		uint32_t size = 0;
		uint32_t padding[3];
		float weight[50] = { 0 };
	}mKernel;
};

#endif // QBlurRenderPass_h__
