#ifndef QParticleSystem_h__
#define QParticleSystem_h__

#include "qvectornd.h"
#include "Render/RHI/QRhiEx.h"
#include "Utils/MathUtils.h"
#include "Render/RHI/QRhiUniformBlock.h"

class IParticleEmitter: public QObject{
	friend class QParticlesRenderComponent;
	Q_OBJECT
	Q_PROPERTY(int NumOfSpawnPerFrame READ getNumOfSpawnPerFrame WRITE setNumOfSpawnPerFrame)
public:
	int getNumOfSpawnPerFrame() const;
	void setNumOfSpawnPerFrame(int inNumOfSpawnPerFrame) ;
protected:
	struct alignas(4) Particle {
		QVector3D position;
		uint32_t _padding;
		QVector3D rotation;
		uint32_t __padding;
		QVector3D scaling = QVector3D(1.0f, 1.0f, 1.0f);
		uint32_t ___padding;
		QVector3D velocity;
		float age = 0.0f;
		float lifetime = 2.0f;
	};

	virtual void setupRhi(QRhiEx* inRhi);
	QRhiBuffer* getTransformBuffer();

	virtual void onTick(QRhiCommandBuffer* inCmdBuffer);
protected:
	QRhiEx* mRhi;
	QScopedPointer<QRhiBuffer> mTransfromBuffer;

	int mNumOfSpawnPerFrame = 100;
	float mDeltaSec = 0.0f;
	float mLastTimeSec = 0.0f;
};

class QCpuParticleEmitter : public IParticleEmitter {
	Q_OBJECT
public:
	inline static const int CPU_PARTICLE_MAX_SIZE = 10000;
	QCpuParticleEmitter();
	uint32_t getNumOfParticle() { return mCurrentParticles.size(); }
protected:
	virtual void onTick(QRhiCommandBuffer* inCmdBuffer) override final;
	virtual void onSpawn(Particle& newParticle) = 0;
	virtual void onUpdate(Particle& outParticle) = 0;
private:
	QVector<Particle> mCurrentParticles;
};

class QGpuParticleEmitter : public IParticleEmitter {
	Q_OBJECT
		Q_PROPERTY(QRhiUniformBlock* SpawnParams READ getSpawnParams)
		Q_PROPERTY(QRhiUniformBlock* UpdateParams READ getUpdateParams)
public:
	inline static const int GPU_PARTICLE_MAX_SIZE = 1000000;
	struct InitParams {
		InitParams();

		QSharedPointer<QRhiUniformBlock> spawnParams;
		QByteArray spawnDefine;
		QByteArray spawnCode;

		QSharedPointer<QRhiUniformBlock> updateParams;
		QByteArray updateDefine;
		QByteArray updateCode;
	};
	void setupParams(const InitParams& params);
	QRhiBuffer* getCurrentIndirectDispatchBuffer();
	QRhiUniformBlock* getSpawnParams() { return mParams.spawnParams.get(); }
	QRhiUniformBlock* getUpdateParams() { return mParams.updateParams.get(); }
private:
	virtual void setupRhi(QRhiEx* inRhi) override;
	virtual void onTick(QRhiCommandBuffer* inCmdBuffer) override final;
	void recompile();
	void onSpawn(QRhiCommandBuffer* inCmdBuffer);
	void onUpdateAndRecyle(QRhiCommandBuffer* inCmdBuffer);
	void onCalcAndSubmitTransform(QRhiCommandBuffer* inCmdBuffer);
private:
	QRhiEx::Signal mSigCompile;
	QRhiEx::Signal mSigInitIndirectBuffer;
	InitParams mParams;
	int mInputSlot = 0;
	int mOutputSlot = 1;

	struct IndirectDispatchBuffer {
		int x, y, z;
	};
	struct UpdateContextBuffer {
		float deltaSec;
		float timestamp;
	};
	QScopedPointer<QRhiBuffer> mUpdateContextBuffer;

	QScopedPointer<QRhiBuffer> mIndirectDispatchBuffer[2];

	QScopedPointer<QRhiBuffer> mParticlesBuffer[2];
	QScopedPointer<QRhiComputePipeline> mSpawnPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mSpawnBindings[2];

	QScopedPointer<QRhiComputePipeline> mUpdatePipeline;
	QScopedPointer<QRhiShaderResourceBindings> mUpdateBindings[2];

	QScopedPointer<QRhiComputePipeline> mTranformComputePipline;
	QScopedPointer<QRhiShaderResourceBindings> mTranformComputeBindings[2];
	QRhiVulkanNativeHandles* mVkHandles = nullptr;
	QVulkanInstance* mVkInstance = nullptr;
	QVulkanDeviceFunctions* mVkDevFunc = nullptr;
};

#endif // QParticleSystem_h__