#ifndef AssetUtils_h__
#define AssetUtils_h__

#include "QGenericMatrix"
#include "assimp/vector3.h"
#include "assimp/matrix4x4.h"
#include "QMatrix4x4"
#include "QDir"
#include "QEngineCoreAPI.h"

class QENGINECORE_API AssetUtils {
public:
	using Mat4 = QGenericMatrix<4, 4, float>;

	static QMatrix4x4 converter(const aiMatrix4x4& aiMat4);

	static QVector3D converter(const aiVector3D& aiVec3);

	static QByteArray loadHdr(const QString& fn, QSize* size);

	static QSize resolveCubeImageFaceSize(const QImage& inImage);

	static std::array<QImage, 6> resolveCubeSubImages(const QImage& inImage);
};

#endif // AssetUtils_h__