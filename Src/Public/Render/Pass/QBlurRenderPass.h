#ifndef QBlurRenderPass_h__
#define QBlurRenderPass_h__

#include "Render/IRenderPass.h"

class QBlurRenderPass: public IRenderPassBase {
	Q_OBJECT
		Q_PROPERTY(int BlurIterations READ getBlurIter WRITE setupBlurIter)
		Q_PROPERTY(int BlurSize READ getBlurSize WRITE setupBlurSize)
		Q_PROPERTY(int DownSampleCount READ getDownSamplerCount WRITE setupDownSample)
	Q_META_BEGIN(QBlurRenderPass)
		Q_META_P_NUMBER_LIMITED(BlurIterations, 0, 10)
		Q_META_P_NUMBER_LIMITED(BlurSize, 1, 40)
		Q_META_P_NUMBER_LIMITED(DownSampleCount, 1, 16)
	Q_META_END()
public:
	QBlurRenderPass();

	enum InSlot {
		Src = 0,
	};

	enum OutSlot {
		Result = 0,
	};

	QBlurRenderPass* setupBlurSize(int size);
	QBlurRenderPass* setupBlurIter(int val);
	QBlurRenderPass* setupDownSample(int val);

	int getBlurIter() const { return mBlurIter; }
	int getBlurSize() const { return mBlurState.size; }
	int getDownSamplerCount() const { return mDownSampleCount; }
	QRhiTextureRenderTarget* getInputRenderTaget() { return mBlurRT[0].renderTarget.get(); }

	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
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
	struct BlurState {
		uint32_t size = 0;
		uint32_t padding[3];
		float weight[50] = { 0 };
	}mBlurState;
	QRhiEx::Signal sigUpdateBlurState;

	int mDownSampleCount = 2;
	int mBlurIter = 2;
};

#endif // QBlurRenderPass_h__
