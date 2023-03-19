#ifndef QSsaoRenderPass_h__
#define QSsaoRenderPass_h__

#include "Render/IRenderPass.h"

class QSsaoRenderPass: public IRenderPassBase {
	Q_OBJECT
		Q_PROPERTY(int SampleSize READ getSampleSize WRITE setupSampleSize)
		Q_PROPERTY(float Radius READ getRadius WRITE setupRadius)
		Q_PROPERTY(float Bias READ getBias WRITE setupBias)
	Q_META_BEGIN(QSsaoRenderPass)
		Q_META_P_NUMBER_LIMITED(SampleSize,1,128)
		Q_META_P_NUMBER_LIMITED(Radius,0,4)
		Q_META_P_NUMBER_LIMITED(Bias, 0.01, 1.0)
	Q_META_END()
public:
	QSsaoRenderPass();

	QSsaoRenderPass* setupSampleSize(int size);
	int getSampleSize() const { return mSsaoState.size; }

	QSsaoRenderPass* setupRadius(float var);
	float getRadius() const { return mSsaoState.radius; }

	QSsaoRenderPass* setupBias(float var);
	float getBias() const { return mSsaoState.bias; }

	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	enum InSlot {
		Position = 0,
		Normal,
	};

	enum OutSlot {
		Result = 0,
	};
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
		float radius = 2.0f;
		float bias = 0.1f;
		uint32_t size = 0;
		uint32_t padding;
		QVector4D samples[128];
		QVector4D noise[16];
		QGenericMatrix<4, 4, float> projection;
	}mSsaoState;
	QRhiEx::Signal sigUpdateSsaoState;
};

#endif // QSsaoRenderPass_h__