#ifndef QStaticMeshRenderComponent_h__
#define QStaticMeshRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"
#include "Asset/QStaticMesh.h"

class QStaticMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QSharedPointer<QStaticMesh> StaticMesh READ getStaticMesh WRITE setStaticMesh)
	Q_PROPERTY(QVector<QSharedPointer<QRhiGraphicsPipelineBuilder>> Pipelines READ getPipelines)

	Q_META_BEGIN(QStaticMeshRenderComponent)
		Q_META_P_ARRAY_FIXED_SIZE(Pipelines, true)
	Q_META_END()

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QStaticMeshRenderComponent)
		Q_BUILDER_ATTRIBUTE(QSharedPointer<QStaticMesh>, StaticMesh)
	Q_BUILDER_END()
public:
	QStaticMeshRenderComponent();
	void setStaticMesh(QSharedPointer<QStaticMesh> val);
	QSharedPointer<QStaticMesh> getStaticMesh() const { return mStaticMesh; }
	const QVector<QSharedPointer<QRhiGraphicsPipelineBuilder>>& getPipelines() const { return mPipelines; }
protected:
	void setupShaderForSubmesh(QRhiGraphicsPipelineBuilder* inPipeline, const QStaticMesh::SubMeshInfo& info);
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QSharedPointer<QStaticMesh> mStaticMesh;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QVector<QSharedPointer<QRhiGraphicsPipelineBuilder>> mPipelines;
};

#endif // QStaticMeshRenderComponent_h__
