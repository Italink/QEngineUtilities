#ifndef QPbrLightingPass_h__
#define QPbrLightingPass_h__

#include "Render/IRenderPass.h"
#include "Render/Painter/TexturePainter.h"
#include "QEngineUtilitiesAPI.h"

class QENGINEUTILITIES_API QPbrLightingPass : public IRenderPass {
	Q_OBJECT
	Q_BUILDER_BEGIN_RENDER_PASS(QPbrLightingPass, Albedo, Position, Normal, Metallic, Roughness, SkyTexture, SkyCube)
	Q_BUILDER_END_RENDER_PASS(FragColor)
protected:
	QRhiEx::Signal sigInit;

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<TexturePainter> mSkyTexturePainter;

	QScopedPointer<QRhiTexture> mPrefilteredSpecularCube;
	QScopedPointer<QRhiBuffer> mPrefilteredSpecularCubeUniformBuffer;
	QScopedPointer<QRhiComputePipeline> mPrefilteredSpecularCubePipeline;
	QScopedPointer<QRhiShaderResourceBindings> mPrefilteredSpecularCubeBindings;

	QScopedPointer<QRhiTexture> mDiffuseIrradianceCube;
	QScopedPointer<QRhiComputePipeline> mDiffuseIrradiancePipeline;
	QScopedPointer<QRhiShaderResourceBindings> mDiffuseIrradianceBindings;

	QScopedPointer<QRhiTexture> mBrdfLut;
	QScopedPointer<QRhiComputePipeline> mBrdfLutPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBrdfLutBindings;

	struct PbrUniformBlock {
		QVector3D eyePosition;
	};
	QScopedPointer<QRhiBuffer> mPbrUniformBlock;
	QScopedPointer<QRhiGraphicsPipeline> mPbrPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mPbrBindings;
};

#endif // QPbrLightingPass_h__