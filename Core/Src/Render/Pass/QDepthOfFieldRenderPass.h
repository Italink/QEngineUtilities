#ifndef QDepthOfFieldRenderPass_h__
#define QDepthOfFieldRenderPass_h__

#include "Render/IRenderPass.h"

class QDepthOfFieldRenderPass: public IRenderPass {
	Q_OBJECT
		Q_PROPERTY(QVector2D ScreenFocusPoint READ getScreenFocusPoint WRITE setScreenFocusPoint)
		Q_PROPERTY(float Near  READ getNear WRITE setNear)
		Q_PROPERTY(float Far READ getFar WRITE setFar)

	Q_META_BEGIN(QDepthOfFieldRenderPass)
		Q_META_P_NUMBER_LIMITED(Near,0, 100)
		Q_META_P_NUMBER_LIMITED(Far,0, 100)
	Q_META_END()

	Q_BUILDER_BEGIN_RENDER_PASS(QDepthOfFieldRenderPass,Focus,LoseFocus,Position)
		Q_BUILDER_ATTRIBUTE(QVector2D, ScreenFocusPoint)
		Q_BUILDER_ATTRIBUTE(float, Near)
		Q_BUILDER_ATTRIBUTE(float, Far)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QDepthOfFieldRenderPass();

	void setScreenFocusPoint(QVector2D val);
	void setNear(float val);
	void setFar(float val);

	QVector2D getScreenFocusPoint() const { return mParams.screenFocusPoint; }
	float getNear() const { return mParams.near; }
	float getFar() const { return mParams.far; }

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QRhiTexture* mSrcTexture = nullptr;
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;

	struct Params {
		QVector2D screenFocusPoint = QVector2D(0.5f, 0.5f);
		float near = 8.0f;
		float far = 12.8f;
	}mParams;
	QRhiEx::Signal sigUpdateParams;
};

#endif // QDepthOfFieldRenderPass_h__
