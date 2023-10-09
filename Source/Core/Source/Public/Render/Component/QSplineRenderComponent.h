#ifndef QSplineRenderComponent_h__
#define QSplineRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/QPrimitiveRenderProxy.h"
#include "Render/RHI/QRhiMaterialGroup.h"

struct QENGINECORE_API QSplinePoint{
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

class QENGINECORE_API QSplineRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(float LineWidth READ getLineWidth WRITE setLineWidth)
	Q_PROPERTY(int SubdivisionAmount READ getSubdivisionAmount WRITE setSubdivisionAmount)
	Q_PROPERTY(QList<QSplinePoint> Points READ getPoints WRITE setPoints)

	Q_CLASSINFO("LineWidth", "Min=0,Max=1000")
	Q_CLASSINFO("SubdivisionAmount", "Min=0,Max=64")
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
protected:
	int mSubdivisionAmount = 16;
	float mLineWidth = 5.0f;

	QList<QSplinePoint> mPoints;
	QList<QVector4D> mSegmentData;
	
	QScopedPointer<QRhiBuffer> mInstanceBuffer;
	QScopedPointer<QRhiBuffer> mVertexBuffer;

	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QSharedPointer<QPrimitiveRenderProxy> mRenderProxy;
};

#endif // QSplineRenderComponent_h__
