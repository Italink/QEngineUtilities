#ifndef QToneMappingRenderPass_h__
#define QToneMappingRenderPass_h__

#include "Render/IRenderPass.h"

class QENGINECORE_API QToneMappingRenderPass :public IRenderPass {
	Q_OBJECT
	Q_PROPERTY(float Gamma READ getGamma WRITE setGamma)
	Q_PROPERTY(float Exposure READ getExposure WRITE setExposure)
	Q_PROPERTY(float PureWhite READ getPureWhite WRITE setPureWhite)

	Q_CLASSINFO("Gamma"		, "Min=0,Max=16")
	Q_CLASSINFO("Exposure"	, "Min=0,Max=16")
	Q_CLASSINFO("PureWhite"	, "Min=0,Max=16")

	Q_BUILDER_BEGIN_RENDER_PASS(QToneMappingRenderPass, Src)
		Q_BUILDER_ATTRIBUTE(float, Gamma)
		Q_BUILDER_ATTRIBUTE(float, Exposure)
		Q_BUILDER_ATTRIBUTE(float, PureWhite)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QToneMappingRenderPass();
	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	float getGamma() const { return mParams.gamma; }
	float getExposure() const { return mParams.exposure; }
	float getPureWhite() const { return mParams.pureWhite; }

	void setGamma(float val);
	void setExposure(float val);
	void setPureWhite(float val);
private:
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct Params {
		float gamma = 2.2f;
		float exposure = 1.0f;
		float pureWhite = 1.0;
	}mParams;
	QRhiEx::Signal sigUpdateParams;
};

#endif // QToneMappingRenderPass_h__
