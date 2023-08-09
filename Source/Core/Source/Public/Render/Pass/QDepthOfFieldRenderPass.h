#ifndef QDepthOfFieldRenderPass_h__
#define QDepthOfFieldRenderPass_h__

#include "Render/IRenderPass.h"

class QENGINECORE_API QDepthOfFieldRenderPass: public IRenderPass {
	Q_OBJECT
	Q_PROPERTY(float Focus WRITE setFocus READ getFocus)
	Q_PROPERTY(float FocalLength WRITE setFocalLength READ getFocalLength)
	Q_PROPERTY(float Aperture WRITE setAperture READ getAperture)
	Q_PROPERTY(int ApertureBlades WRITE setApertureBlades READ getApertureBlades)
	Q_PROPERTY(float BokehSqueeze WRITE setBokehSqueeze READ getBokehSqueeze)
	Q_PROPERTY(float BokehSqueezeFalloff WRITE setBokehSqueezeFalloff READ getBokehSqueezeFalloff)
	Q_PROPERTY(int Iterations WRITE setIterations READ getIterations)

	Q_CLASSINFO("Focus", "Min=0,Max=0.3")
	Q_CLASSINFO("FocalLength", "Min=0.01,Max=256")
	Q_CLASSINFO("Aperture", "Min=0,Max=128")
	Q_CLASSINFO("ApertureBlades", "Min=3,Max=16")
	Q_CLASSINFO("BokehSqueeze", "Min=-3,Max=0")
	Q_CLASSINFO("BokehSqueezeFalloff", "Min=0,Max=10")
	Q_CLASSINFO("Iterations", "Min=0,Max=256")

	Q_BUILDER_BEGIN_RENDER_PASS(QDepthOfFieldRenderPass,Src,Position)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QDepthOfFieldRenderPass();

	void setFocus(float val);
	void setAperture(float val);
	void setFocalLength(float val);
	void setApertureBlades(int val);
	void setBokehSqueeze(float val);
	void setBokehSqueezeFalloff(float val);
	void setIterations(int val);


	float getFocus() const { return mParams.focus; }
	float getAperture() const { return mParams.aperture; }
	float getFocalLength() const { return mParams.focalLength; }
	int getApertureBlades() const { return mParams.apertureBlades; }
	float getBokehSqueeze() const { return mParams.bokehSqueeze; }
	float getBokehSqueezeFalloff() const { return mParams.bokehSqueezeFalloff; }
	int getIterations() const { return mParams.iterations; }

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
		QGenericMatrix<4, 4, float> VP;
		float focus = 0.05f;
		float focalLength = 20.0f;
		float aperture = 2.5f;
		int apertureBlades = 5;
		float bokehSqueeze = 0.0f;
		float bokehSqueezeFalloff = 1.0f;
		float aspectRatio = 1.77f;
		int iterations = 64;
	}mParams;
	QRhiEx::Signal mSigUpdateParams;
};

#endif // QDepthOfFieldRenderPass_h__