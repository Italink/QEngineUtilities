#ifndef QDynamicMeshRenderComponent_h__
#define QDynamicMeshRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/QPrimitiveRenderProxy.h"

class QENGINECORE_API QDynamicMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QRhiMaterialGroup* Materials READ getMaterialGroup)
public:
	QDynamicMeshRenderComponent();

	QRhiMaterialGroup* getMaterialGroup() { return mMaterialGroup.get(); }
protected:
	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D texCoord;
	};
	virtual void onUpdateVertices(QVector<Vertex>& vertices) = 0;
	void onRebuildResource() override;
protected:
	QVector<Vertex> mVertices;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QSharedPointer<QPrimitiveRenderProxy> mRenderProxy;
	QScopedPointer<QRhiMaterialGroup> mMaterialGroup;
};

#endif // QDynamicMeshRenderComponent_h__
