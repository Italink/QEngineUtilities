#ifndef MathUtils_h__
#define MathUtils_h__

#include "QGenericMatrix"
#include "qvectornd.h"
#include "QMatrix4x4"
#include "QEngineCoreAPI.h"

class QENGINECORE_API MathUtils {
public:
	using Mat4 = QGenericMatrix<4, 4, float>;

	static void setMatTranslate(QMatrix4x4& mat4, QVector3D translate);
	static void setMatRotation(QMatrix4x4& mat4, QVector3D rotation);
	static void setMatScale3D(QMatrix4x4& mat4, QVector3D scale3D);
	static void setMatOrthoNormalize(QMatrix4x4& mat4);

	static QVector3D getMatTranslate(const QMatrix4x4& mat4);
	static QVector3D getMatRotation(const QMatrix4x4& mat4);
	static QVector3D getMatScale3D(const QMatrix4x4& mat4);
};

#endif // MathUtils_h__
