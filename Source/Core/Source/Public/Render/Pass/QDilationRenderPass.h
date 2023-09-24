//#ifndef QDilationRenderPass_h__
//#define QDilationRenderPass_h__
//
//#include "Render/IRenderPass.h"
//
//class QENGINECORE_API QDilationRenderPass: public IRenderPass {
//	Q_OBJECT
//	Q_PROPERTY(int DilationSize READ getDilationSize WRITE setDilationSize)
//	Q_PROPERTY(float DilationSeparation  READ getDilationSeparation WRITE setDilationSeparation)
//	Q_PROPERTY(float MinThreshold READ getMinThreshold WRITE setMinThreshold)
//	Q_PROPERTY(float MaxThreshold READ getMaxThreshold WRITE setMaxThreshold)
//
//	Q_CLASSINFO("DilationSize", "Min=1,Max=40")
//	Q_CLASSINFO("DilationSeparation", "Min=0.01,Max=10")
//	Q_CLASSINFO("MinThreshold", "Min=0,Max=1")
//	Q_CLASSINFO("MaxThreshold", "Min=0,Max=1")
//
//
//	Q_BUILDER_BEGIN_RENDER_PASS(QDilationRenderPass,Src)
//		Q_BUILDER_ATTRIBUTE(int, DilationIterations)
//		Q_BUILDER_ATTRIBUTE(float, DilationSize)
//		Q_BUILDER_ATTRIBUTE(float, MinThreshold)
//		Q_BUILDER_ATTRIBUTE(float, MaxThreshold)
//	Q_BUILDER_END_RENDER_PASS(Result)
//public:
//	QDilationRenderPass();
//
//	void setDilationSize(int size);
//	void setDilationSeparation(float val);
//	void setMinThreshold(float val);
//	void setMaxThreshold(float val);
//
//	int getDilationSize() const { return mParams.size; }
//	float getDilationSeparation() const { return mParams.separation; }
//	float getMinThreshold() const { return mParams.minThreshold; }
//	float getMaxThreshold() const { return mParams.maxThreshold; }
//
//	void resizeAndLinkNode(const QSize& size) override;
//	void compile() override;
//	void render(QRhiCommandBuffer* cmdBuffer) override;
//private:
//	QRhiTexture* mSrcTexture = nullptr;
//	QScopedPointer<QRhiSampler> mSampler;
//	QScopedPointer<QRhiBuffer> mUniformBuffer;
//	struct DilationRT {
//		QScopedPointer<QRhiTexture> colorAttachment;
//		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
//	};
//	DilationRT mDilationRT[2];
//	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
//	QScopedPointer<QRhiGraphicsPipeline> mPipelineH;
//	QScopedPointer<QRhiGraphicsPipeline> mPipelineV;
//
//	QScopedPointer<QRhiShaderResourceBindings> mBindingsH;
//	QScopedPointer<QRhiShaderResourceBindings> mBindingsV;
//
//	struct Params {
//		uint32_t size = 5;
//		float separation = 1.0f;
//		float minThreshold = 0.2f;
//		float maxThreshold = 0.5f;
//	}mParams;
//	QRhiSignal sigUpdateParams;
//};
//
//#endif // QDilationRenderPass_h__
