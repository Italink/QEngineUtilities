#ifndef QMotionBlurRenderPass_h__
#define QMotionBlurRenderPass_h__

#include "Render/IRenderPass.h"

class QENGINECORE_API QMotionBlurRenderPass: public IRenderPass {
	Q_OBJECT
	Q_PROPERTY(int MotionBlurSize READ getMotionBlurSize WRITE setMotionBlurSize)
	Q_PROPERTY(float MotionBlurSeparation READ getMotionBlurSeparation WRITE setMotionBlurSeparation)

	//Q_CLASSINFO("MotionBlurIterations", "Min=0,Max=10")
	//Q_CLASSINFO("MotionBlurSize", "Min=1,Max=40")
	//Q_CLASSINFO("DownSampleCount", "Min=1,Max=16")

	Q_BUILDER_BEGIN_RENDER_PASS(QMotionBlurRenderPass,BaseColor,Position)
		Q_BUILDER_ATTRIBUTE(int, MotionBlurSize)
		Q_BUILDER_ATTRIBUTE(float, MotionBlurSeparation)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QMotionBlurRenderPass();

	void setMotionBlurSize(int size);
	void setMotionBlurSeparation(float val);

	int getMotionBlurSize() const { return mParams.size; }
	float getMotionBlurSeparation() const { return mParams.separation; }

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct MotionBlurRT {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	};
	MotionBlurRT mMotionBlurRT;
	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	struct Params {
		QGenericMatrix<4, 4, float> previousViewToWorld;
		QGenericMatrix<4, 4, float> worldToView;
		QGenericMatrix<4, 4, float> projection;
		uint32_t size = 0;
		float separation;
	}mParams;
	QRhiEx::Signal sigUpdateParams;
};

#endif // QMotionBlurRenderPass_h__
