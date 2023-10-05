#ifndef QStaticMeshRenderComponent_h__
#define QStaticMeshRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/QPrimitiveRenderProxy.h"
#include "Render/RHI/QRhiMaterialGroup.h"
#include "Asset/QStaticMesh.h"
#include "QEngineUtilitiesAPI.h"

class QENGINEUTILITIES_API QStaticMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QSharedPointer<QStaticMesh> StaticMesh READ getStaticMesh WRITE setStaticMesh)
	Q_PROPERTY(QRhiMaterialGroup* Materials READ getMaterialGroup)

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QStaticMeshRenderComponent)
		Q_BUILDER_ATTRIBUTE(QSharedPointer<QStaticMesh>, StaticMesh)
	Q_BUILDER_END()
public:
	QStaticMeshRenderComponent();
	void setStaticMesh(QSharedPointer<QStaticMesh> val);
	QSharedPointer<QStaticMesh> getStaticMesh() const;
	QRhiMaterialGroup* getMaterialGroup();
protected:
	void onRebuildResource() override;
protected:
	QSharedPointer<QStaticMesh> mStaticMesh;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QVector<QSharedPointer<QPrimitiveRenderProxy>> mProxies;
	QScopedPointer<QRhiMaterialGroup> mMaterialGroup;
};

#endif // QStaticMeshRenderComponent_h__
