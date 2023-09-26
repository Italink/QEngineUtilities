#ifndef QSkyPassBuilder_h__
#define QSkyPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QSkyPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QSkyPassBuilder)
		QRP_INPUT_ATTR(QString, SkyImagePath);
		QRP_INPUT_ATTR(QImage, SkyImage);
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

	IRenderer* mRenderer = nullptr;
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

	QRhiComputePipelineRef mSkyCubePipeline;
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


//#ifndef QSkyRenderPass_h__
//#define QSkyRenderPass_h__
//
//#include "Render/IRenderPass.h"
//
//class QENGINECORE_API QSkyRenderPass : public IRenderPass {
//	Q_OBJECT
//	Q_PROPERTY(QImage SkyboxImage READ getSkyBoxImage WRITE setSkyBoxImage);
//	
//	Q_BUILDER_BEGIN_RENDER_PASS_WITHOUT_IN(QSkyRenderPass)
//		Q_BUILDER_FUNCTION_BEGIN(setSkyBoxImagePath, const QString& inPath)
//			Q_BUILDER_OBJECT_PTR-RefsetSkyBoxImagePath(inPath);
//		Q_BUILDER_FUNCTION_END()
//	Q_BUILDER_END_RENDER_PASS(Equirect, SkyTexture, SkyCube)
//private:

//	struct RTResource {
//		QRhiTextureRef colorAttachment;
//		QRhiTextureRenderTargetRef renderTarget;
//		QRhiRenderPassDescriptorRef renderPassDesc;
//	};
//	RTResource mRT;

//public:
//	QSkyRenderPass();
//
//	void setSkyBoxImagePath(const QString& inPath);
//	void setSkyBoxImage(QImage inImage);
//	QImage getSkyBoxImage() const;
//protected:
//	void resizeAndLinkNode(const QSize& size) override;
//	void compile() override;
//	void render(QRhiCommandBuffer* cmdBuffer) override;
//};
// 
//#endif // QSkyRenderPass_h__