#ifndef QDilationRenderPass_h__
#define QDilationRenderPass_h__

#include "Render/IRenderPass.h"

class QDilationRenderPass: public IRenderPass {
	Q_OBJECT
		Q_PROPERTY(int DilationSize READ getDilationSize WRITE setDilationSize)
		Q_PROPERTY(float DilationSeparation  READ getDilationSeparation WRITE setDilationSeparation)
		Q_PROPERTY(float MinThreshold READ getMinThreshold WRITE setMinThreshold)
		Q_PROPERTY(float MaxThreshold READ getMaxThreshold WRITE setMaxThreshold)

	Q_META_BEGIN(QDilationRenderPass)
		Q_META_P_NUMBER_LIMITED(DilationSize, 1, 40)
		Q_META_P_NUMBER_LIMITED(DilationSeparation, 0.01, 10)
		Q_META_P_NUMBER_LIMITED(MinThreshold,0, 1)
		Q_META_P_NUMBER_LIMITED(MinThreshold,0, 1)
	Q_META_END()

	Q_BUILDER_BEGIN_RENDER_PASS(QDilationRenderPass,Src)
		Q_BUILDER_ATTRIBUTE(int, DilationIterations)
		Q_BUILDER_ATTRIBUTE(float, DilationSize)
		Q_BUILDER_ATTRIBUTE(float, MinThreshold)
		Q_BUILDER_ATTRIBUTE(float, MaxThreshold)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QDilationRenderPass();

	void setDilationSize(int size);
	void setDilationSeparation(float val);
	void setMinThreshold(float val);
	void setMaxThreshold(float val);

	int getDilationSize() const { return mParams.size; }
	float getDilationSeparation() const { return mParams.separation; }
	float getMinThreshold() const { return mParams.minThreshold; }
	float getMaxThreshold() const { return mParams.maxThreshold; }

	QRhiTextureRenderTarget* getInputRenderTaget() { return mDilationRT[0].renderTarget.get(); }

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QRhiTexture* mSrcTexture = nullptr;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct DilationRT {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	};
	DilationRT mDilationRT[2];
	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineH;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineV;

	QScopedPointer<QRhiShaderResourceBindings> mBindingsH;
	QScopedPointer<QRhiShaderResourceBindings> mBindingsV;

	struct Params {
		uint32_t size = 5;
		float separation = 1;
		float minThreshold = 0.2;
		float maxThreshold = 0.5;
	}mParams;
	QRhiEx::Signal sigUpdateParams;
};

#endif // QDilationRenderPass_h__
