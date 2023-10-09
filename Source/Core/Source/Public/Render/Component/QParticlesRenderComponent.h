#ifndef QParticlesRenderComponent_h__
#define QParticlesRenderComponent_h__

#include "Asset/QParticleEmitter.h"
#include "Asset/QStaticMesh.h"
#include "Render/ISceneRenderComponent.h"
#include "Type/QColor4D.h"
#include "Render/QPrimitiveRenderProxy.h"

class QENGINECORE_API QParticlesRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QSharedPointer<QStaticMesh> ParticleShape READ getParticleShape WRITE setParticleShape)
		Q_PROPERTY(QRhiMaterialGroup* Materials READ getMaterialGroup)
		Q_PROPERTY(IParticleEmitter* Emitter READ getEmitter WRITE setEmitter)
		Q_PROPERTY(bool FacingCamera READ getFacingCamera WRITE setFacingCamera)

		Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QParticlesRenderComponent)
			Q_BUILDER_ATTRIBUTE(IParticleEmitter*, Emitter)
			Q_BUILDER_ATTRIBUTE(QSharedPointer<QStaticMesh>, ParticleShape)
			Q_BUILDER_ATTRIBUTE(bool, FacingCamera)
		Q_BUILDER_END()
public:
	QParticlesRenderComponent();

	void setEmitter(IParticleEmitter* inEmitter);
	IParticleEmitter* getEmitter() const { return mEmitter.get(); }

	void setParticleShape(QSharedPointer<QStaticMesh> inStaticMesh);
	QSharedPointer<QStaticMesh> getParticleShape();
	void setFacingCamera(bool val);
	bool getFacingCamera() const;
	QRhiMaterialGroup* getMaterialGroup() const { return mMaterialGroup.get(); }
protected:
	void onPreRenderTick(QRhiCommandBuffer* cmdBuffer) override;
	void onRebuildResource() override;
protected:
	QSharedPointer<IParticleEmitter> mEmitter;
	QScopedPointer<QRhiBuffer> mIndirectDrawBuffer;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QSharedPointer<QPrimitiveRenderProxy> mRenderProxy;
	QScopedPointer<QRhiMaterialGroup> mMaterialGroup;

	struct IndirectDrawBuffer {
		uint32_t indexCount;
		uint32_t instanceCount = 1;
		uint32_t firstIndex;
		int32_t vertexOffset;
		uint32_t firstInstance;
	};

	bool bFacingCamera = false;
	QSharedPointer<QStaticMesh> mStaticMesh;
};

#endif // QParticlesRenderComponent_h__
