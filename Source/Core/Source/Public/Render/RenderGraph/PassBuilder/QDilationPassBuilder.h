#ifndef QDilationPassBuilder_h__
#define QDilationPassBuilder_h__

#include "IRenderPassBuilder.h"

class QENGINECORE_API QDilationPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QDilationPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(int, DilationSize) = 2.2f;
		QRP_INPUT_ATTR(float, DilationSeparation) = 1.0f;
		QRP_INPUT_ATTR(float, MinThreshold) = 1.0f;
		QRP_INPUT_ATTR(float, MaxThreshold) = 1.0f;
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QDilationPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, DilationReslut);
	QRP_OUTPUT_END()
public:
	QDilationPassBuilder();
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mDilationFSH;
	QShader mDilationFSV;
	struct DilationRT {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock {
		uint32_t size = 5;
		float separation = 1.0f;
		float minThreshold = 0.2f;
		float maxThreshold = 0.5f;
	}mParams;
	DilationRT mDilationRT[2];
	QRhiBufferRef mUniformBuffer;
	QRhiSamplerRef mSampler;
	QRhiGraphicsPipelineRef mPipelineH;
	QRhiShaderResourceBindingsRef mBindingsH;
	QRhiGraphicsPipelineRef mPipelineV;
	QRhiShaderResourceBindingsRef mBindingsV;
};

#endif