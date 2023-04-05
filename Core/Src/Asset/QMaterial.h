#ifndef QMaterial_h__
#define QMaterial_h__

#include "QList"
#include "QMap"
#include "QSharedPointer"

struct aiScene;

struct QMaterial {
	static QVector<QSharedPointer<QMaterial>> CreateFromScene(const aiScene* scene, QString modelPath);

	QMap<QString, QVariant> mProperties;
};


#endif // QMaterial_h__
