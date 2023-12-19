#pragma once

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QTexutrePassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QTexutrePassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(QRhiRenderTarget*, DstRenderTarget);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QTexutrePassBuilder)
	QRP_OUTPUT_END()
public:
	QTexutrePassBuilder();
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mFragmentShader;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mShaderBindings;		
	QRhiGraphicsPipelineRef mPipeline;		
};