#ifndef QParticlesRenderComponent_h__
#define QParticlesRenderComponent_h__

#include "Render/ISceneRenderComponent.h"
#include "Asset/QParticleEmitter.h"
#include "Type/QColor4D.h"

class QParticlesRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
		Q_PROPERTY(IParticleEmitter* Emitter READ getEmitter WRITE setEmitter)
		Q_PROPERTY(bool FacingCamera READ getFacingCamera WRITE setFacingCamera)

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QParticlesRenderComponent)
		Q_BUILDER_ATTRIBUTE(IParticleEmitter*, Emitter)
		Q_BUILDER_ATTRIBUTE(bool, FacingCamera)
	Q_BUILDER_END()
public:
	QParticlesRenderComponent();

	void setEmitter(IParticleEmitter* inEmitter);
	IParticleEmitter* getEmitter() const { return mEmitter.get(); }

	void setFacingCamera(bool val);
	bool getFacingCamera() const;
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onPreUpdate(QRhiCommandBuffer* cmdBuffer) override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QSharedPointer<IParticleEmitter> mEmitter;
	QScopedPointer<QRhiBuffer> mIndirectDrawBuffer;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;

	struct IndirectDrawBuffer {
		quint32 vertexCount;
		quint32 instanceCount = 1;
		quint32 firstVertex = 0;
		quint32 firstInstance = 0;
	};
	struct UniformBlock {
		QGenericMatrix<4, 4, float> M;
		QGenericMatrix<4, 4, float> V;
		QGenericMatrix<4, 4, float> P;
		QColor4D Color = QColor4D(1.0f,1.0f,1.0f);
	}mUniform;
	bool bFacingCamera = true;
};

#endif // QParticlesRenderComponent_h__
