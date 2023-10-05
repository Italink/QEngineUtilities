#include "QParticleEmitter.h"
#include "QDateTime"
#include "private/qrhivulkan_p.h"
#include "qvulkanfunctions.h"
#include "private/qvulkandefaultinstance_p.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "Render/RHI/Vulkan/QRhiVulkanExHelper.h"

int IParticleEmitter::getNumOfSpawnPerFrame() const {
	return mNumOfSpawnPerFrame;
}

void IParticleEmitter::setNumOfSpawnPerFrame(int inNumOfSpawnPerFrame) {
	mNumOfSpawnPerFrame = inNumOfSpawnPerFrame;
}

void IParticleEmitter::setupRhi(QRhi* inRhi) {
	mRhi = inRhi;
}

QRhiBuffer* IParticleEmitter::getTransformBuffer() {
	return mTransfromBuffer.get();
}

void IParticleEmitter::onTick(QRhiCommandBuffer* inCmdBuffer) {
	float currentSecond = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
	if (mLastTimeSec == 0.0f)
		mLastTimeSec = currentSecond;
	mDeltaSec = currentSecond - mLastTimeSec;
	mLastTimeSec = currentSecond;
}

QCpuParticleEmitter::QCpuParticleEmitter() {
	mCurrentParticles.reserve(CPU_PARTICLE_MAX_SIZE);
}

void QCpuParticleEmitter::onTick(QRhiCommandBuffer* inCmdBuffer) {
	IParticleEmitter::onTick(inCmdBuffer);

	if (mTransfromBuffer.isNull()) {
		mTransfromBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer, sizeof(MathUtils::Mat4) * CPU_PARTICLE_MAX_SIZE));
		mTransfromBuffer->setName("TransfromBuffer");
		mTransfromBuffer->create();
	}

	{	// spawn
		int numOfSpawn = qMin(CPU_PARTICLE_MAX_SIZE - mCurrentParticles.size(), mNumOfSpawnPerFrame);
		QVector<IParticleEmitter::Particle> newParticles(numOfSpawn);
		for (int i = 0; i < newParticles.size(); i++) {
			onSpawn(newParticles[i]);
		}
		mCurrentParticles.append(newParticles);
	}

	{	//update
		int outIndex = 0;
		for (int inIndex = 0; inIndex < mCurrentParticles.size(); inIndex++) {
			Particle particle = mCurrentParticles[inIndex];
			if (particle.age < particle.lifetime) {
				onUpdate(particle);
				particle.age += mDeltaSec;
				particle.position += particle.velocity;
				mCurrentParticles[outIndex] = particle;
				++outIndex;
			}
		}
		mCurrentParticles.resize(outIndex);
	}
	QVector<MathUtils::Mat4> transforms(mCurrentParticles.size());
	for (int i = 0; i < mCurrentParticles.size(); i++) {
		const Particle& particle = mCurrentParticles[i];
		QMatrix4x4 mat;
		mat.translate(particle.position);
		mat.rotate(particle.rotation.x(), QVector3D(1, 0, 0));
		mat.rotate(particle.rotation.y(), QVector3D(0, 1, 0));
		mat.rotate(particle.rotation.z(), QVector3D(0, 0, 1));
		mat.scale(particle.scaling);
		transforms[i] = mat.toGenericMatrix<4, 4>();
	}
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mTransfromBuffer.get(), 0, sizeof(MathUtils::Mat4) * transforms.size(), transforms.data());
	inCmdBuffer->resourceUpdate(batch);
}


QGpuParticleEmitter::InitParams::InitParams() {
	spawnParams.reset(new QRhiUniformBlock);
	spawnParams->setObjectName("Params");

	spawnDefine = R"(	
		float rand(float seed, float min, float max){
			return min + (max-min) * fract(sin(dot(vec2(gl_GlobalInvocationID.x * seed * UpdateCtx.deltaSec,UpdateCtx.timestamp) ,vec2(12.9898,78.233))) * 43758.5453);
		}
	)";

	spawnCode = R"(		
		outParticle.age = 0.0f;
		outParticle.lifetime = 2.0f;
		outParticle.scaling = vec3(0.1,0.1,0.1);
		outParticle.position =  vec3(0,0,0);
		outParticle.velocity = vec3(rand(0.72131231,-0.1,0.1),rand(0.07891,-0.1,0.1),0.2);
	)";	

	updateParams.reset(new QRhiUniformBlock);
	updateParams->setObjectName("Params");

	updateCode = R"(		
		outParticle.age	 = inParticle.age + UpdateCtx.deltaSec;
		outParticle.position = inParticle.position + inParticle.velocity;
		outParticle.velocity = inParticle.velocity;
		outParticle.scaling  = inParticle.scaling;
		outParticle.rotation = inParticle.rotation;
	)";
}

void QGpuParticleEmitter::setupParams(const InitParams& params) {
	mParams = params;
	mSigCompile.request();
}

QRhiBuffer* QGpuParticleEmitter::getCurrentIndirectDispatchBuffer() {
	return mIndirectDispatchBuffer[mInputSlot].get();
}

void QGpuParticleEmitter::setupRhi(QRhi* inRhi) {
	IParticleEmitter::setupRhi(inRhi);
	mVkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
	mVkInstance = QRhiVulkanExHelper::instance();
	mVkDevFunc = mVkInstance->deviceFunctions(mVkHandles->dev);

	mSigCompile.request();
}

void QGpuParticleEmitter::recompile() {
	mTransfromBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(float) * 16 * GPU_PARTICLE_MAX_SIZE));
	mTransfromBuffer->setName("TransfromBuffer");
	mTransfromBuffer->create();

	mIndirectDispatchBuffer[0].reset(QRhiHelper::newVkBuffer(mRhi, QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(float) * 3));
	mIndirectDispatchBuffer[0]->setName("IndirectDispatchBuffer0");
	mIndirectDispatchBuffer[0]->create();
	mIndirectDispatchBuffer[1].reset(QRhiHelper::newVkBuffer(mRhi, QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(float) * 3));
	mIndirectDispatchBuffer[1]->setName("IndirectDispatchBuffer1");
	mIndirectDispatchBuffer[1]->create();

	mParticlesBuffer[0].reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(IParticleEmitter::Particle) * GPU_PARTICLE_MAX_SIZE));
	mParticlesBuffer[0]->setName("ParticlesBuffer0");
	mParticlesBuffer[0]->create();
	mParticlesBuffer[1].reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(IParticleEmitter::Particle) * GPU_PARTICLE_MAX_SIZE));
	mParticlesBuffer[1]->setName("ParticlesBuffer1");
	mParticlesBuffer[1]->create();
	mUpdateContextBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::UniformBuffer, sizeof(UpdateContextBuffer)));
	mUpdateContextBuffer->setName("UpdateContextBuffer");
	mUpdateContextBuffer->create();

	mParams.spawnParams->create(mRhi);
	QVector<QRhiShaderResourceBinding> spawnBindings;
	spawnBindings << QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[0].get());
	spawnBindings << QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	spawnBindings << QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::ComputeStage, mUpdateContextBuffer.get());
	if (!mParams.spawnParams->isEmpty()) {
		spawnBindings << QRhiShaderResourceBinding::uniformBuffer(3, QRhiShaderResourceBinding::ComputeStage, mParams.spawnParams->getUniformBlock());
	}
	mSpawnBindings[0].reset(mRhi->newShaderResourceBindings());
	mSpawnBindings[0]->setBindings(spawnBindings.begin(), spawnBindings.end());
	mSpawnBindings[0]->create();
	spawnBindings[0] = QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[1].get());
	spawnBindings[1] = QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	mSpawnBindings[1].reset(mRhi->newShaderResourceBindings());
	mSpawnBindings[1]->setBindings(spawnBindings.begin(), spawnBindings.end());
	mSpawnBindings[1]->create();

	mSpawnPipeline.reset(mRhi->newComputePipeline());
	mSpawnPipeline->setShaderResourceBindings(mSpawnBindings[mInputSlot].get());
	QShader spawnShader = QRhiHelper::newShaderFromCode( QShader::Stage::ComputeStage, QString(R"(
		#version 450
		#define GPU_PARTICLE_MAX_SIZE 1000000
		struct Particle {
			vec3 position;
			vec3 rotation;
			vec3 scaling;
			vec3 velocity;
			float age;
			float lifetime;
		};
		layout(std140,binding = 0) buffer IndirectBuffer{
			ivec3 dispatch;
		};
		layout(std140,binding = 1) buffer ParticlesBuffer{
			Particle particles[GPU_PARTICLE_MAX_SIZE];
		};
		layout(std140,binding = 2) uniform UpdateContextBuffer{
			float deltaSec;
			float timestamp;
		}UpdateCtx;
		%1
		#define outParticle particles[ID]
		%2
		void main(){
			uint ID = atomicAdd(dispatch.x,1);
			%3
		})")
		.arg(mParams.spawnParams->createDefineCode(3))
		.arg(mParams.spawnDefine)
		.arg(mParams.spawnCode)
		.toLocal8Bit()
	);
	mSpawnPipeline->setShaderStage({ QRhiShaderStage::Compute,spawnShader });
	mSpawnPipeline->create();

	mParams.updateParams->create(mRhi);

	QVector<QRhiShaderResourceBinding> updateBindings;
	updateBindings << QRhiShaderResourceBinding::bufferStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[1].get());
	updateBindings << QRhiShaderResourceBinding::bufferLoad(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	updateBindings << QRhiShaderResourceBinding::bufferStore(2, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	updateBindings << QRhiShaderResourceBinding::uniformBuffer(3, QRhiShaderResourceBinding::ComputeStage, mUpdateContextBuffer.get());
	if (!mParams.updateParams->isEmpty()) {
		updateBindings << QRhiShaderResourceBinding::uniformBuffer(4, QRhiShaderResourceBinding::ComputeStage, mParams.updateParams->getUniformBlock());
	}
	mUpdateBindings[0].reset(mRhi->newShaderResourceBindings());
	mUpdateBindings[0]->setBindings(updateBindings.begin(), updateBindings.end());
	mUpdateBindings[0]->create();

	updateBindings[0] = QRhiShaderResourceBinding::bufferStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[0].get());
	updateBindings[1] = QRhiShaderResourceBinding::bufferLoad(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	updateBindings[2] = QRhiShaderResourceBinding::bufferStore(2, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	mUpdateBindings[1].reset(mRhi->newShaderResourceBindings());
	mUpdateBindings[1]->setBindings(updateBindings.begin(), updateBindings.end());
	mUpdateBindings[1]->create();

	mUpdatePipeline.reset(mRhi->newComputePipeline());
	mUpdatePipeline->setShaderResourceBindings(mUpdateBindings[mInputSlot].get());
	QShader updateShader = QRhiHelper::newShaderFromCode( QShader::Stage::ComputeStage, QString(R"(#version 450
		#define GPU_PARTICLE_MAX_SIZE 1000000
		struct Particle {
			vec3 position;
			vec3 rotation;
			vec3 scaling;
			vec3 velocity;
			float age;
			float lifetime;
		};
		layout(std140,binding = 0) buffer OutputIndirectBuffer{
			ivec3 outputDispatch;
		};
		layout(std140,binding = 1) buffer InputParticlesBuffer{
			Particle inParticles[GPU_PARTICLE_MAX_SIZE];
		};
		layout(std140,binding = 2) buffer OutputParticlesBuffer{
			Particle outParticles[GPU_PARTICLE_MAX_SIZE];
		};
		layout(std140,binding = 3) uniform UpdateContextBuffer{
			float deltaSec;
			float timestamp;
		}UpdateCtx;
		%1
		#define inParticle inParticles[inID]
		#define outParticle outParticles[outID]
		%2
		void main(){
			uint inID = gl_GlobalInvocationID.x;
			int isAlive = inParticles[inID].age < inParticles[inID].lifetime ? 1 : 0;
			uint outID = atomicAdd(outputDispatch.x,isAlive);
			outID = max(GPU_PARTICLE_MAX_SIZE*(1-isAlive),outID);
			outParticle.lifetime = inParticle.lifetime;
			%3
		})")
		.arg(mParams.updateParams->createDefineCode(4))
		.arg(mParams.updateDefine)
		.arg(mParams.updateCode)
		.toLocal8Bit()
	);
	mUpdatePipeline->setShaderStage({ QRhiShaderStage::Compute,updateShader });
	mUpdatePipeline->create();

	mTranformComputeBindings[0].reset(mRhi->newShaderResourceBindings());
	mTranformComputeBindings[0]->setBindings({
		QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get()),
		QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage,mTransfromBuffer.get()),
	});
	mTranformComputeBindings[0]->create();
	mTranformComputeBindings[1].reset(mRhi->newShaderResourceBindings());
	mTranformComputeBindings[1]->setBindings({
		QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get()),
		QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage,mTransfromBuffer.get()),
	});
	mTranformComputeBindings[1]->create();

	mTranformComputePipline.reset(mRhi->newComputePipeline());
	mTranformComputePipline->setShaderResourceBindings(mTranformComputeBindings[0].get());
	QShader matrixCompute = QRhiHelper::newShaderFromCode( QShader::ComputeStage, R"(
		#version 450
		#define GPU_PARTICLE_MAX_SIZE 1000000
		struct Particle {
			vec3 position;
			vec3 rotation;
			vec3 scaling;
			vec3 velocity;
			float age;
			float lifetime;
		};
		layout(std140,binding = 0) buffer InputParticle{
			Particle inParticles[GPU_PARTICLE_MAX_SIZE];
		};

		layout(std140,binding = 1) buffer OutputParticle{
			mat4 outMatrix[GPU_PARTICLE_MAX_SIZE];
		};

		void main(){
			const uint index = gl_GlobalInvocationID.x ;      //根据工作单元的位置换算出内存上的索引
			vec3 position = inParticles[index].position;
			vec3 rotation  = inParticles[index].rotation;
			vec3 scale    = inParticles[index].scaling;

			mat4 matScale = mat4(scale.x,0,0,0,
								 0,scale.y,0,0,
								 0,0,scale.z,0,
								 0,0,0,1);

	        vec2 X=vec2(sin(rotation.x),cos(rotation.x));
            vec2 Y=vec2(sin(rotation.y),cos(rotation.y));
            vec2 Z=vec2(sin(rotation.z),cos(rotation.z));

			mat4 matRotation=mat4(Z.y*Y.y,Z.y*Y.x*X.x-Z.x*X.y,Z.y*Y.x*X.y-Z.x*X.x,0,
                                  Z.x*Y.y,Z.x*Y.x*X.x+Z.y*X.y,Z.x*Y.x*X.y-Z.y*X.x,0,
                                  -Y.x,Y.y*X.x,Y.y*X.y,0,
                                  0,0,0,1
                                  );
			mat4 matTranslate=mat4(1,0,0,0,
								0,1,0,0,
								0,0,1,0,
								position.x,position.y,position.z,1);
			outMatrix[index] = matTranslate * matScale * matRotation;
		}
	)");
	mTranformComputePipline->setShaderStage({ QRhiShaderStage::Compute,matrixCompute });
	mTranformComputePipline->create();

	mSigInitIndirectBuffer.request();
}

void QGpuParticleEmitter::onTick(QRhiCommandBuffer* inCmdBuffer) {
	IParticleEmitter::onTick(inCmdBuffer);
	if (mSigCompile.ensure()) {
		recompile();
	}
	if (mSigInitIndirectBuffer.ensure()) {
		IndirectDispatchBuffer dispatch = { 0,1,1 };
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->uploadStaticBuffer(mIndirectDispatchBuffer[0].get(), &dispatch);
		batch->uploadStaticBuffer(mIndirectDispatchBuffer[1].get(), &dispatch);
		inCmdBuffer->resourceUpdate(batch);
		mRhi->finish();
	}
	if (mParams.spawnParams->sigRecreateBuffer.ensure()) {
		mSigCompile.request();
	}
	if (mParams.updateParams->sigRecreateBuffer.ensure()) {
		mSigCompile.request();
	}

	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	mParams.spawnParams->updateResource(batch);
	mParams.updateParams->updateResource(batch);
	inCmdBuffer->resourceUpdate(batch);

	onSpawn(inCmdBuffer);
	onUpdateAndRecyle(inCmdBuffer);
	onCalcAndSubmitTransform(inCmdBuffer);
	qSwap(mInputSlot, mOutputSlot);
}

void QGpuParticleEmitter::onSpawn(QRhiCommandBuffer* inCmdBuffer) {
	QRhiVulkanCommandBufferNativeHandles* cmdBufferHandles = (QRhiVulkanCommandBufferNativeHandles*)inCmdBuffer->nativeHandles();
	VkCommandBuffer cmdBuffer = cmdBufferHandles->commandBuffer;
	QVkComputePipeline* pipelineHandles = (QVkComputePipeline*)mSpawnPipeline.get();
	QVkShaderResourceBindings* bindingsHandles = (QVkShaderResourceBindings*)mSpawnBindings[mInputSlot].get();
	QRhiVulkan* rhi = *(QRhiVulkan**)(mRhi);
	auto buffer = mIndirectDispatchBuffer[mInputSlot]->nativeBuffer();
	VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];

	UpdateContextBuffer updateCtx;
	updateCtx.deltaSec = mDeltaSec;
	updateCtx.timestamp = QTime::currentTime().msecsSinceStartOfDay() / 1000000.0f;
	char* p = mUpdateContextBuffer->beginFullDynamicBufferUpdateForCurrentFrame();
	memcpy(p, &updateCtx, sizeof(UpdateContextBuffer));
	mUpdateContextBuffer->endFullDynamicBufferUpdateForCurrentFrame();

	inCmdBuffer->beginExternal();

	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	barrier.buffer = vkBuffer;
	barrier.offset = 0;
	barrier.pNext = nullptr;
	barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = -1;
	barrier.size = mIndirectDispatchBuffer[mInputSlot]->size();
	mVkDevFunc->vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

	mVkDevFunc->vkCmdBindPipeline(cmdBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, pipelineHandles->pipeline);
	QRhiVulkanExHelper::setShaderResources(mSpawnPipeline.get(), inCmdBuffer, mSpawnBindings[mInputSlot].get());
	mVkDevFunc->vkCmdDispatch(cmdBuffer, mNumOfSpawnPerFrame, 1, 1);

	inCmdBuffer->endExternal();
}

void QGpuParticleEmitter::onUpdateAndRecyle(QRhiCommandBuffer* inCmdBuffer) {
	QRhiVulkanCommandBufferNativeHandles* cmdBufferHandles = (QRhiVulkanCommandBufferNativeHandles*)inCmdBuffer->nativeHandles();
	VkCommandBuffer cmdBuffer = cmdBufferHandles->commandBuffer;
	QVkComputePipeline* pipelineHandles = (QVkComputePipeline*)mUpdatePipeline.get();
	QRhiVulkan* rhi = *(QRhiVulkan**)(mRhi);

	auto buffer = mIndirectDispatchBuffer[mInputSlot]->nativeBuffer();
	VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];

	auto outputBuffer = mIndirectDispatchBuffer[mOutputSlot]->nativeBuffer();
	VkBuffer outputVkBuffer = *(VkBuffer*)outputBuffer.objects[0];

	IndirectDispatchBuffer dispatch = { 0,1,1 };
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->uploadStaticBuffer(mIndirectDispatchBuffer[mOutputSlot].get(), &dispatch);
	inCmdBuffer->resourceUpdate(batch);

	inCmdBuffer->beginExternal();

	VkBufferMemoryBarrier barrier[2];
	barrier[0].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier[0].srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	barrier[0].buffer = outputVkBuffer;
	barrier[0].offset = 0;
	barrier[0].pNext = nullptr;
	barrier[0].srcQueueFamilyIndex = barrier[0].dstQueueFamilyIndex = -1;
	barrier[0].size = mIndirectDispatchBuffer[mOutputSlot]->size();

	barrier[1].sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier[1].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	barrier[1].dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	barrier[1].buffer = vkBuffer;
	barrier[1].offset = 0;
	barrier[1].pNext = nullptr;
	barrier[1].srcQueueFamilyIndex = barrier[1].dstQueueFamilyIndex = -1;
	barrier[1].size = mIndirectDispatchBuffer[mInputSlot]->size();

	mVkDevFunc->vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &barrier[0], 0, nullptr);
	mVkDevFunc->vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, nullptr, 1, &barrier[1], 0, nullptr);

	mVkDevFunc->vkCmdBindPipeline(cmdBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, pipelineHandles->pipeline);
	QRhiVulkanExHelper::setShaderResources(mUpdatePipeline.get(), inCmdBuffer, mUpdateBindings[mInputSlot].get());
	mVkDevFunc->vkCmdDispatchIndirect(cmdBuffer, vkBuffer, 0);

	inCmdBuffer->endExternal();
}

void QGpuParticleEmitter::onCalcAndSubmitTransform(QRhiCommandBuffer* inCmdBuffer) {
	QRhiVulkanCommandBufferNativeHandles* cmdBufferHandles = (QRhiVulkanCommandBufferNativeHandles*)inCmdBuffer->nativeHandles();
	VkCommandBuffer cmdBuffer = cmdBufferHandles->commandBuffer;
	QVkComputePipeline* pipelineHandles = (QVkComputePipeline*)mTranformComputePipline.get();
	QRhiVulkan* rhi = *(QRhiVulkan**)(mRhi);
	auto buffer = mIndirectDispatchBuffer[mOutputSlot]->nativeBuffer();
	VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];

	inCmdBuffer->beginExternal();

	VkBufferMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	barrier.buffer = vkBuffer;
	barrier.offset = 0;
	barrier.pNext = nullptr;
	barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = -1;
	barrier.size = mIndirectDispatchBuffer[mOutputSlot]->size();
	mVkDevFunc->vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);

	mVkDevFunc->vkCmdBindPipeline(cmdBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, pipelineHandles->pipeline);
	QRhiVulkanExHelper::setShaderResources(mTranformComputePipline.get(), inCmdBuffer, mTranformComputeBindings[mOutputSlot].get());
	mVkDevFunc->vkCmdDispatchIndirect(cmdBuffer, vkBuffer, 0);

	inCmdBuffer->endExternal();
}
