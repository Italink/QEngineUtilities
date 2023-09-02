#ifndef QStaticMesh_h__
#define QStaticMesh_h__

#include "qvectornd.h"
#include "QImage"
#include "QMap"
#include "QMatrix4x4"
#include "QMaterial.h"
#include "QFont"
#include "QEngineCoreAPI.h"

struct QENGINECORE_API QStaticMesh {

	enum Shape {
		Plane,
		Cube,
		Sphere,
	};

	static QSharedPointer<QStaticMesh> CreateFromFile(const QString& inFilePath);

	static QSharedPointer<QStaticMesh> CreateFromImage(QImage image);

	static QSharedPointer<QStaticMesh> CreateFromShape(Shape shape);

	static QSharedPointer<QStaticMesh> CreateFromText(
		const QString& inText,
		const QFont& inFont,
		QColor inColor = Qt::white,
		Qt::Orientation o = Qt::Horizontal,
		int inSpacing = 2,
		bool bUseTexture = true,
		float lod = 1.0f
	);


	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D uv;
	};

	struct SubMeshData {
		uint32_t verticesOffset;
		uint32_t verticesRange;
		uint32_t indicesOffset;
		uint32_t indicesRange;
		uint32_t materialIndex = 0;
		QMatrix4x4 localTransfrom;
	};
	using Index = uint32_t;

	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QVector<SubMeshData> mSubmeshes;
	QVector<QSharedPointer<QMaterial>> mMaterials;
};

Q_DECLARE_METATYPE(QSharedPointer<QStaticMesh>)

#endif // QStaticMesh_h__
