#ifndef QDynamicMeshRenderComponent_h__
#define QDynamicMeshRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"

class QDynamicMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QSharedPointer<QRhiGraphicsPipelineBuilder> Pipeline READ getPipeline)
public:
	QDynamicMeshRenderComponent();
	QSharedPointer<QRhiGraphicsPipelineBuilder> getPipeline() const { return mPipeline; }
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
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QVector<Vertex> mVertices;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QSharedPointer<QRhiGraphicsPipelineBuilder> mPipeline;
};

#endif // QDynamicMeshRenderComponent_h__
