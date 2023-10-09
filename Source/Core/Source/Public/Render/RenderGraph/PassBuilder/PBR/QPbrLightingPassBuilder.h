#ifndef QPbrLightingPassBuilder_h__
#define QPbrLightingPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
#include "Render/RenderGraph/Painter/TexturePainter.h"
#include "Render/RHI/QRhiHelper.h"

class QENGINECORE_API QPbrLightingPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QPbrLightingPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColor);
		QRP_INPUT_ATTR(QRhiTextureRef, Position);
		QRP_INPUT_ATTR(QRhiTextureRef, Normal);
		QRP_INPUT_ATTR(QRhiTextureRef, Metallic);
		QRP_INPUT_ATTR(QRhiTextureRef, Roughness);
		QRP_INPUT_ATTR(QRhiTextureRef, SkyTexture);
		QRP_INPUT_ATTR(QRhiTextureRef, SkyCube);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QPbrLightingPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, LightingResult)
	QRP_OUTPUT_END()
public:
	QPbrLightingPassBuilder();
protected:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QRhiSignal mSigGeneratePbrTexture;

	QRhiTextureRenderTargetRef mRenderTarget;

	QScopedPointer<TexturePainter> mSkyTexturePainter;
	QShader mPrefilteredCS;
	QRhiTextureRef mPrefilteredSpecularCube;
	QRhiBufferRef mPrefilteredSpecularCubeUniformBuffer;
	QRhiComputePipelineRef mPrefilteredSpecularCubePipeline;
	QVector<QRhiShaderResourceBindingsRef> mPrefilteredSpecularCubeBindings;

	QShader mDiffuseIrradianceCS;
	QRhiTextureRef mDiffuseIrradianceCube;
	QRhiComputePipelineRef mDiffuseIrradiancePipeline;
	QRhiShaderResourceBindingsRef mDiffuseIrradianceBindings;

	QShader mBrdfLutCS;
	QRhiTextureRef mBrdfLut;
	QRhiComputePipelineRef mBrdfLutPipeline;
	QRhiShaderResourceBindingsRef mBrdfLutBindings;

	QShader mPbrFS;
	struct PbrUniformBlock {
		QVector3D eyePosition;
	};
	QRhiBufferRef mPbrUniformBlock;
	QRhiGraphicsPipelineRef mPbrPipeline;
	QRhiSamplerRef mSampler;
	QRhiSamplerRef mMipmapSampler;
	QRhiShaderResourceBindingsRef mPbrBindings;
};

#endif // QPbrLightingPassBuilder_h__