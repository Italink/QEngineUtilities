#ifndef QSkyRenderPass_h__
#define QSkyRenderPass_h__

#include "Render/IRenderPass.h"

class QSkyRenderPass : public IRenderPass {
	Q_OBJECT
	Q_PROPERTY(QImage SkyboxImage READ getSkyBoxImage WRITE setSkyBoxImage);
	
	Q_BUILDER_BEGIN_RENDER_PASS_WITHOUT_IN(QSkyRenderPass)
		Q_BUILDER_FUNCTION_BEGIN(setSkyBoxImagePath, const QString& inPath)
			Q_BUILDER_OBJECT_PTR->setSkyBoxImagePath(inPath);
		Q_BUILDER_FUNCTION_END()
	Q_BUILDER_END_RENDER_PASS(Preview, SkyCube, SpecularCube, IrradianceCube, BrdfLut)
private:
	QRhiEx::Signal sigInitResource;
	QRhiEx::Signal sigUploadEquirectTexture;
	QRhiEx::Signal sigComputeSkyCube;
	QRhiEx::Signal sigComputeIBLCube;
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

	bool bIsEquirectangular = false;
	QByteArray mImageData;
	QImage mSkyBoxImage;

	QScopedPointer<QRhiTexture> mEquirectTexture;

	QScopedPointer<QRhiTexture> mSkyCube;
	QScopedPointer<QRhiComputePipeline> mSkyCubePipeline;
	QScopedPointer<QRhiShaderResourceBindings> mSkyCubeBindings;

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
public:
	QSkyRenderPass();

	void setSkyBoxImagePath(const QString& inPath);
	void setSkyBoxImage(QImage inImage);
	QImage getSkyBoxImage() const;
protected:
	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
};
#endif // QSkyRenderPass_h__