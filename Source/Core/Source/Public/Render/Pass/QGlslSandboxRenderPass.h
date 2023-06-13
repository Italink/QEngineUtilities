#ifndef QGlslSandboxRenderPass_h__
#define QGlslSandboxRenderPass_h__

#include "Render/IRenderPass.h"

class QENGINECORE_API QGlslSandboxRenderPass : public IRenderPass {
	Q_OBJECT
	Q_PROPERTY(QString ShaderCode READ getShaderCode WRITE setShaderCode)

	Q_BUILDER_BEGIN_RENDER_PASS_WITHOUT_IN(QGlslSandboxRenderPass)
		Q_BUILDER_ATTRIBUTE(QString, ShaderCode)
	Q_BUILDER_END_RENDER_PASS(Output)
private:
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
public:
	QString getShaderCode() const { return mRawShaderCode; }
	void setShaderCode(QString val);
protected:
	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	QString mShaderCode;
	QString mRawShaderCode;
	QRhiEx::Signal sigRecompile;

	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiBuffer> mUniformBlock;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;

	struct UniformBlock {
		QVector2D mouse;
		QVector2D resolution;
		QVector2D sufaceSize;
		float time;
	};
};
#endif // QGlslSandboxRenderPass_h__