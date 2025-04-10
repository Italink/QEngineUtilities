#ifndef QGaussianSplattingPointCloud_h__
#define QGaussianSplattingPointCloud_h__

#include <QVector3D>
#include <QMatrix4x4>
#include <QQuaternion>
#include "QEngineCoreAPI.h"

struct QENGINECORE_API QGaussianSplattingPoint {
	QVector4D Position;
	QVector4D Color;
	QGenericMatrix<4, 4, float> Sigma;
};

struct QENGINECORE_API QGaussianSplattingPointCloud {

	static QSharedPointer<QGaussianSplattingPointCloud> CreateFromFile(const QString& inFilePath);

	QVector<QGaussianSplattingPoint> mPoints;
	QPair<QVector3D, QVector3D> mBounds;
};

Q_DECLARE_METATYPE(QSharedPointer<QGaussianSplattingPointCloud>)

#endif // QGaussianSplattingPointCloud_h__
