#ifndef QSkeletalMeshRenderComponent_h__
#define QSkeletalMeshRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/QPrimitiveRenderProxy.h"
#include "Asset/QSkeletalMesh.h"
#include "QEngineUtilitiesAPI.h"

class QENGINEUTILITIES_API QSkeletalMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QSharedPointer<QSkeletalMesh> SkeletalMesh READ getSkeletalMesh WRITE setSkeletalMesh)
	Q_PROPERTY(QRhiMaterialGroup* MaterialGroup READ getMaterialGroup)

	Q_CLASSINFO("SkeletalMeshPath", "Type=FilePath")

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QSkeletalMeshRenderComponent)
		Q_BUILDER_ATTRIBUTE(QSharedPointer<QSkeletalMesh>, SkeletalMesh)
	Q_BUILDER_END()
public:
	QSkeletalMeshRenderComponent();
	void setSkeletalMesh(QSharedPointer<QSkeletalMesh> val);
	QSharedPointer<QSkeletalMesh> getSkeletalMesh() const { return mSkeletalMesh; }
	QRhiMaterialGroup* getMaterialGroup() { return mMaterialGroup.get(); }
protected:
	void onRebuildResource() override;
protected:
	QSharedPointer<QSkeletalMesh> mSkeletalMesh;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QSharedPointer<QRhiUniformBlock> mUniformBlock;
	QVector<QSharedPointer<QPrimitiveRenderProxy>> mPipelines;
	QScopedPointer<QRhiMaterialGroup> mMaterialGroup;
};

#endif // QSkeletalMeshRenderComponent_h__
