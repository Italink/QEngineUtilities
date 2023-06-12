#ifndef QSsaoRenderPass_h__
#define QSsaoRenderPass_h__

#include "Render/IRenderPass.h"

class QSsaoRenderPass: public IRenderPass {
	Q_OBJECT
		Q_PROPERTY(int SampleSize READ getSampleSize WRITE setupSampleSize)
		Q_PROPERTY(float Radius READ getRadius WRITE setupRadius)
		Q_PROPERTY(float Bias READ getBias WRITE setupBias)

	//Q_META_BEGIN(QSsaoRenderPass)
	//	Q_META_P_NUMBER_LIMITED(SampleSize,1,128)
	//	Q_META_P_NUMBER_LIMITED(Radius,0,4)
	//	Q_META_P_NUMBER_LIMITED(Bias, 0.01, 1.0)
	//Q_META_END()

	Q_BUILDER_BEGIN_RENDER_PASS(QSsaoRenderPass, Position, Normal)
		Q_BUILDER_ATTRIBUTE(int, SampleSize)
		Q_BUILDER_ATTRIBUTE(float, Radius)
		Q_BUILDER_ATTRIBUTE(float, Bias)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QSsaoRenderPass();

	QSsaoRenderPass* setupSampleSize(int size);
	int getSampleSize() const { return mSsaoState.size; }

	QSsaoRenderPass* setupRadius(float var);
	float getRadius() const { return mSsaoState.radius; }

	QSsaoRenderPass* setupBias(float var);
	float getBias() const { return mSsaoState.bias; }

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

private:
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct RT {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	};
	RT mRT;
	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;

	struct SsaoState {
		QGenericMatrix<4, 4, float> VP;
		float radius = 2.0f;
		float bias = 0.1f;
		uint32_t size = 0;
		uint32_t padding;
		QVector4D samples[128];
		QVector4D noise[16];
	}mSsaoState;
	QRhiEx::Signal sigUpdateSsaoState;
};

#endif // QSsaoRenderPass_h__