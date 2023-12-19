#ifndef QGlslSandboxPassBuilder_h__
#define QGlslSandboxPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API QGlslSandboxPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QGlslSandboxPassBuilder)
		QRP_INPUT_ATTR(QString, ShaderCode);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QGlslSandboxPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, GlslSandboxResult);
	QRP_OUTPUT_END()
public:
	QGlslSandboxPassBuilder(){}
protected:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QShader mGlslSandboxFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock {
		QVector2D mouse;
		QVector2D resolution;
		QVector2D sufaceSize;
		float time;
	};
	RTResource mRT;
	QRhiBufferRef mUniformBuffer;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiShaderResourceBindingsRef mBindings;
	QString mShaderCode;
};

#endif