#ifndef QBlurRenderPass_h__
#define QBlurRenderPass_h__

#include "Render/IRenderPass.h"

class QBlurRenderPass: public IRenderPass {
	Q_OBJECT
		Q_PROPERTY(int BlurIterations READ getBlurIter WRITE setBlurIter)
		Q_PROPERTY(int BlurSize READ getBlurSize WRITE setBlurSize)
		Q_PROPERTY(int DownSampleCount READ getDownSamplerCount WRITE setDownSample)

	Q_META_BEGIN(QBlurRenderPass)
		Q_META_P_NUMBER_LIMITED(BlurIterations, 0, 10)
		Q_META_P_NUMBER_LIMITED(BlurSize, 1, 40)
		Q_META_P_NUMBER_LIMITED(DownSampleCount, 1, 16)
	Q_META_END()

	Q_BUILDER_BEGIN_RENDER_PASS(QBlurRenderPass,Src)
		Q_BUILDER_ATTRIBUTE(int, BlurIterations)
		Q_BUILDER_ATTRIBUTE(int, BlurSize)
		Q_BUILDER_ATTRIBUTE(int, DownSampleCount)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QBlurRenderPass();

	void setBlurSize(int size);
	void setBlurIter(int val);
	void setDownSample(int val);

	int getBlurIter() const { return mBlurIter; }
	int getBlurSize() const { return mParams.size; }
	int getDownSamplerCount() const { return mDownSampleCount; }
	QRhiTextureRenderTarget* getInputRenderTaget() { return mBlurRT[0].renderTarget.get(); }

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QRhiTexture* mSrcTexture = nullptr;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct BlurRT {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	};
	BlurRT mBlurRT[2];
	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineDownSample;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineH;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineV;

	QScopedPointer<QRhiShaderResourceBindings> mBindingsDownSample;
	QScopedPointer<QRhiShaderResourceBindings> mBindingsH;
	QScopedPointer<QRhiShaderResourceBindings> mBindingsV;
	struct Params {
		uint32_t size = 0;
		uint32_t padding[3];
		float weight[50] = { 0 };
	}mParams;
	QRhiEx::Signal sigUpdateParams;

	int mDownSampleCount = 2;
	int mBlurIter = 2;
};

#endif // QBlurRenderPass_h__
