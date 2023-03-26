#ifndef QSkyboxRenderComponent_h__
#define QSkyboxRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"

class QSkyboxRenderComponent : public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QImage SkyboxImage READ getSkyBoxImage WRITE setSkyBoxImage);

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QSkyboxRenderComponent)
		Q_BUILDER_ATTRIBUTE(QImage, SkyboxImage)
	Q_BUILDER_END()
public:
	QSkyboxRenderComponent();
	void setSkyBoxImage(QImage inImage);
	QImage getSkyBoxImage() const;
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onPreUpdate(QRhiCommandBuffer* cmdBuffer) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QScopedPointer<QRhiGraphicsPipelineBuilder> mPipeline;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QImage mSkyBoxImage;
};

#endif // QSkyboxRenderComponent_h__