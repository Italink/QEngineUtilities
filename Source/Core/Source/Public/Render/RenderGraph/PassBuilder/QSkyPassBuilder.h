#ifndef QSkyPassBuilder_h__
#define QSkyPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QSkyPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QSkyPassBuilder)
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QSkyPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, Equirect);
		QRP_OUTPUT_ATTR(QRhiTextureRef, SkyTexture);
		QRP_OUTPUT_ATTR(QRhiTextureRef, SkyCube);
	QRP_OUTPUT_END()
public:
	QSkyPassBuilder();
	void setSkyBoxImageByPath(const QString& inPath);
	void setSkyBoxImage(QImage inImage);
private:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mSkyBoxVS;
	QShader mSkyBoxFS;
	QShader mSkyBoxGenCS;

	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	RTResource mRT;
	QRhiBufferRef mVertexBuffer;
	QRhiSamplerRef mSampler;

	bool bIsEquirectangular = false;
	QByteArray mImageData;
	QImage mSkyBoxImage;

	QRhiComputePipelineRef mSkyCubeGenPipeline;
	QRhiShaderResourceBindingsRef mSkyCubeBindings;

	struct SkyboxUniformBlock {
		QGenericMatrix<4, 4, float> MVP;
	};
	QRhiBufferRef mSkyboxUniformBlock;
	QRhiBufferRef mSkyboxVertexBuffer;
	QRhiGraphicsPipelineRef mSkyboxPipeline;
	QRhiShaderResourceBindingsRef mSkyboxBindings;

	QRhiSignal mSigUploadSkyboxVertics;
	QRhiSignal mSigUploadEquirectTexture;
	QRhiSignal mSigComputeSkyCube;
};

#endif // QSkyPassBuilder_h__
