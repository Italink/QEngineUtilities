#ifndef QSplineRenderComponent_h__
#define QSplineRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"
#include "Render/RHI/QRhiMaterialGroup.h"

struct QSplinePoint{
	Q_GADGET
	Q_PROPERTY(QVector3D Point READ getPoint WRITE setPoint)
	Q_PROPERTY(QColor4D Color READ getColor WRITE setColor)
public:
	QSplinePoint(float x = 0, float y = 0, float z = 0) 
		:QSplinePoint(QVector3D(x,y,z))
	{}

	QSplinePoint(QVector3D inPoint, QColor4D inColor = QColor4D(1, 1, 1, 1))
		: mPoint(inPoint)
		, __padding(0)
		, mColor(inColor)
	{
	}

	QVector3D getPoint() const { return mPoint; }
	void setPoint(QVector3D val) { mPoint = val; }

	QColor4D getColor() const { return mColor; }
	void setColor(QColor4D val) { mColor = val; }

	QVector3D mPoint;
	float __padding;
	QColor4D mColor;
};

class QSplineRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(float LineWidth READ getLineWidth WRITE setLineWidth)
	Q_PROPERTY(int SubdivisionAmount READ getSubdivisionAmount WRITE setSubdivisionAmount)
	Q_PROPERTY(QList<QSplinePoint> Points READ getPoints WRITE setPoints)

	Q_META_BEGIN(QSplineRenderComponent)
		Q_META_P_NUMBER_LIMITED(LineWidth, 0, 1000)
		Q_META_P_NUMBER_LIMITED(SubdivisionAmount, 0, 64)
	Q_META_END()
	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QSplineRenderComponent)
		Q_BUILDER_ATTRIBUTE(QList<QSplinePoint>, Points)
	Q_BUILDER_END()
public:
	QSplineRenderComponent();

	float getLineWidth() const;
	void setLineWidth(float val);

	QList<QSplinePoint> getPoints() const;
	void setPoints(QList<QSplinePoint> val);

	int getSubdivisionAmount() const;
	void setSubdivisionAmount(int val);
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	int mSubdivisionAmount = 16;

	QList<QSplinePoint> mPoints;
	QList<QVector4D> mSegmentData;
	
	QScopedPointer<QRhiBuffer> mInstancingBuffer;
	QScopedPointer<QRhiBuffer> mVertexBuffer;

	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipelineBuilder> mPipelineBuilder;
};

#endif // QSplineRenderComponent_h__
