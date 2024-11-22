#ifndef QSkeletalMesh_h__
#define QSkeletalMesh_h__

#include "QSharedPointer"
#include "qvectornd.h"
#include "QImage"
#include "QMap"
#include "QMatrix4x4"
#include "Utils/MathUtils.h"
#include "QVariantAnimation"
#include "QMaterial.h"
#include "QEngineCoreAPI.h"

struct QENGINECORE_API QSkeleton {
	struct MeshNode {
		QString name;
		QMatrix4x4 localTransform;
		QVector<QSharedPointer<MeshNode>> children;
	};
	struct BoneNode {
		uint16_t index;
		QString name;
		QMatrix4x4 transformOffset;
	};
	QSharedPointer<MeshNode> mMeshRoot;
	QHash<QString, QSharedPointer<BoneNode>> mBoneMap;
	QVector<MathUtils::Mat4> mBoneOffsetMatrix;
};

struct QENGINECORE_API QSkeletalAnimation {
	struct AnimNode {
		QMap<double, QVector3D> translation;
		QMap<double, QQuaternion> rotation;
		QMap<double, QVector3D> scaling;

		QMatrix4x4 getMatrix(const double& timeMs);
	};

	QMap<QString, AnimNode> mAnimNode;
	double mDuration;
	double mTicksPerSecond;
};

class QENGINECORE_API QSkeletalMesh {
public:
	static QSharedPointer<QSkeletalMesh> CreateFromFile(const QString& inFilePath);
	void resetPoses();
	void playAnimation(int inAnimIndex);
protected:
	void processBoneTransform(QSkeletalAnimation* inAnim, double inTimeMs);
public:
	using Index = uint32_t;
	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D texCoord;
		uint32_t boneIndex[4] = { 0,0,0,0 };
		QVector4D boneWeight;
	};

	struct SubMeshData {
		uint32_t verticesOffset;
		uint32_t verticesRange;
		uint32_t indicesOffset;
		uint32_t indicesRange;
		uint32_t materialIndex;
	};

	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QVector<SubMeshData> mSubmeshes;
	QVector<QSharedPointer<QMaterial>> mMaterials;
	QSharedPointer<QVariantAnimation> mAnimPlayer;
	QSharedPointer<QSkeleton> mSkeleton;
	QVector<MathUtils::Mat4> mCurrentPosesMatrix;
	QVector<QSharedPointer<QSkeletalAnimation>> mAnimations;
};

#endif // QSkeletalMesh_h__
