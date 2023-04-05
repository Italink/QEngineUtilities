#ifndef QPbrLightingPass_h__
#define QPbrLightingPass_h__

#include "Render/IRenderPass.h"

class QPbrLightingPass : public IRenderPass {
	Q_OBJECT
	Q_BUILDER_BEGIN_RENDER_PASS(QPbrLightingPass, Albedo, Position, Normal, Metallic, Roughness, SkyCube, SpecularCube, IrradianceCube, BrdfLut)
	Q_BUILDER_END_RENDER_PASS(FragColor)
protected:
	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	QRhiEx::Signal sigUploadSkyboxVertics;

	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;

	struct SkyboxUniformBlock {
		QGenericMatrix<4,4,float> MVP;
	};
	QScopedPointer<QRhiBuffer> mSkyboxUniformBlock;
	QScopedPointer<QRhiBuffer> mSkyboxVertexBuffer;
	QScopedPointer<QRhiGraphicsPipeline> mSkyboxPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mSkyboxBindings;

	struct PbrUniformBlock {
		QVector3D eyePosition;
	};
	QScopedPointer<QRhiBuffer> mPbrUniformBlock;
	QScopedPointer<QRhiGraphicsPipeline> mPbrPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mPbrBindings;
};

#endif // QPbrLightingPass_h__