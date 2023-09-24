#pragma once

#include "Render/RenderGraph/IRGPassBuilder.h"

class QENGINECORE_API QTexutreCopyPassBuilder : public::IRGPassBuilder {
public:
	QTexutreCopyPassBuilder();
	struct InputParams {
		QRhiTextureRef SrcTexture;
		QRhiRenderTarget* DstRenderTarget;
	}Input;

	struct OutputParams {

	}Output;
public:
	void setup(QRGBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mVertexShader;
	QShader mFragmentShader;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mShaderBindings;		
	QRhiGraphicsPipelineRef mPipeline;		
};