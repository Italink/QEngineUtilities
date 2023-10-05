#ifndef QPixelFilterPassBuilder_h__
#define QPixelFilterPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QPixelFilterPassBuilder : public::IRenderPassBuilder {
public:
	QPixelFilterPassBuilder();

	QRP_INPUT_BEGIN(QPixelFilterPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(QString, FilterCode);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QPixelFilterPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, FilterResult);
	QRP_OUTPUT_END()
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mFilterFS;
	QString mFilterCode;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	RTResource mRT;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QPixelFilterPassBuilder_h__