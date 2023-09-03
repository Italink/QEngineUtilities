#ifndef QOutliningRenderPass_h__
#define QOutliningRenderPass_h__

#include "Render/IRenderPass.h"
#include "Type/QColor4D.h"

class QENGINECORE_API QOutliningRenderPass: public IRenderPass {
	Q_OBJECT
	Q_PROPERTY(float MinSeparation READ getMinSeparation WRITE setMinSeparation)
	Q_PROPERTY(float MaxSeparation READ getMaxSeparation WRITE setMaxSeparation)
	Q_PROPERTY(float MinDistance READ getMinDistance WRITE setMinDistance)
	Q_PROPERTY(float MaxDistance READ getMaxDistance WRITE setMaxDistance)
	Q_PROPERTY(int Radius READ getRadius WRITE setRadius)
	Q_PROPERTY(QColor4D ColorModifier READ getColorModifier WRITE setColorModifier)

	Q_CLASSINFO("MinSeparation", "Min=0,Max=10")
	Q_CLASSINFO("MaxSeparation", "Min=1,Max=10")
	Q_CLASSINFO("MinDistance", "Min=0,Max=10")
	Q_CLASSINFO("MaxDistance", "Min=0,Max=10")
	Q_CLASSINFO("Radius", "Min=1,Max=8")

	Q_BUILDER_BEGIN_RENDER_PASS(QOutliningRenderPass,BaseColor,Position)
		Q_BUILDER_ATTRIBUTE(float, MinSeparation)
		Q_BUILDER_ATTRIBUTE(float, MaxSeparation)
		Q_BUILDER_ATTRIBUTE(float, MinDistance)
		Q_BUILDER_ATTRIBUTE(float, MaxDistance)
		Q_BUILDER_ATTRIBUTE(int, Radius)
		Q_BUILDER_ATTRIBUTE(QColor4D, ColorModifier)
	Q_BUILDER_END_RENDER_PASS(Result)
public:
	QOutliningRenderPass();

	void setMinSeparation(float val) { mParams.MinSeparation = val; }
	void setMaxSeparation(float val) { mParams.MaxSeparation = val; }
	void setMinDistance(float val) { mParams.MinDistance = val; }
	void setMaxDistance(float val) { mParams.MaxDistance = val; }
	void setRadius(int val) { mParams.Radius = val; }
	void setColorModifier(QColor4D val) { mParams.ColorModifier = val; }

	float getMinSeparation() const { return mParams.MinSeparation; }
	float getMaxSeparation() const { return mParams.MaxSeparation; }
	float getMinDistance() const { return mParams.MinDistance; }
	float getMaxDistance() const { return mParams.MaxDistance; }
	int getRadius() const { return mParams.Radius; }
	QColor4D getColorModifier() const { return mParams.ColorModifier; }

	void resizeAndLinkNode(const QSize& size) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct OutliningRT {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	};
	OutliningRT mOutliningRT;
	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;

	struct Params {
		QGenericMatrix<4, 4, float> VP;
		float MinSeparation = 1.0f;
		float MaxSeparation = 3.0f;
		float MinDistance = 0.5f;
		float MaxDistance = 2.0f ;
		float FarNear;
		int Radius = 2;
		alignas(16) QColor4D ColorModifier = QColor4D(0.324f,0.063f,0.099f,1.0f);
	}mParams;
	QRhiEx::Signal sigUpdateParams;
};

#endif // QOutliningRenderPass_h__
