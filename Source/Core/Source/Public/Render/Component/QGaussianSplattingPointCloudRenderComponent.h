#ifndef QGaussianSplattingPointCloudRenderComponent_h__
#define QGaussianSplattingPointCloudRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Render/QPrimitiveRenderProxy.h"
#include "Render/RHI/QRhiMaterialGroup.h"
#include "Asset/QGaussianSplattingPointCloud.h"

class QENGINECORE_API QGaussianSplattingPointCloudRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QSharedPointer<QGaussianSplattingPointCloud> GaussianSplattingPointCloud READ getGaussianSplattingPointCloud WRITE setGaussianSplattingPointCloud)

		Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QGaussianSplattingPointCloudRenderComponent)
		Q_BUILDER_ATTRIBUTE(QSharedPointer<QGaussianSplattingPointCloud>, GaussianSplattingPointCloud)
		Q_BUILDER_END()
public:
	QGaussianSplattingPointCloudRenderComponent();
	void setGaussianSplattingPointCloud(QSharedPointer<QGaussianSplattingPointCloud> val);
	QSharedPointer<QGaussianSplattingPointCloud> getGaussianSplattingPointCloud() const;
protected:
	void onPreRenderTick(QRhiCommandBuffer* cmdBuffer) override;
	void onRebuildResource() override;

	void onCpuSort(QRhiCommandBuffer* cmdBuffer);
protected:
	QSharedPointer<QGaussianSplattingPointCloud> mGaussianSplattingPointCloud;
	QScopedPointer<QRhiBuffer> mQuadBuffer;
	QScopedPointer<QRhiBuffer> mGSBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QSharedPointer<QPrimitiveRenderProxy> mRenderProxy;
};

#endif // QGaussianSplattingPointCloudRenderComponent_h__
