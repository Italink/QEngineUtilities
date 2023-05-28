#include "Render/Component/QParticlesRenderComponent.h"
#include "Render/IRenderPass.h"
#include "QVulkanInstance"
#include "qvulkanfunctions.h"
#include "Utils/DebugUtils.h"
#include "private/qvulkandefaultinstance_p.h"

static float ParticleShape[] = {
	 0.01f,   0.01f,
	-0.01f,   0.01f,
	 0.01f,  -0.01f,
	-0.01f,  -0.01f,
};

QParticlesRenderComponent::QParticlesRenderComponent() {
}

void QParticlesRenderComponent::setEmitter(IParticleEmitter* inEmitter) {
	mEmitter.reset(inEmitter);
	mSigRebuildResource.request();
	mSigRebuildPipeline.request();
}

void QParticlesRenderComponent::setFacingCamera(bool val) {
	bFacingCamera = val;
	mSigRebuildPipeline.request();
}

bool QParticlesRenderComponent::getFacingCamera() const {
	return bFacingCamera;
}

void QParticlesRenderComponent::onRebuildResource() {
	mEmitter->setupRhi(mRhi);

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer, sizeof(ParticleShape)));
	mVertexBuffer->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UsageFlag::UniformBuffer, sizeof(UniformBlock)));
	mUniformBuffer->create();

	if (mEmitter && mEmitter->metaObject()->inherits(&QGpuParticleEmitter::staticMetaObject)) {
		mIndirectDrawBuffer.reset(mRhi->newVkBuffer(QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, sizeof(IndirectDrawBuffer)));
		mIndirectDrawBuffer->create();
		mIndirectDrawBuffer->setName("IndirectDrawBuffer");
	}
}

void QParticlesRenderComponent::onRebuildPipeline() {
	mSigRebuildPipeline.ensure();

	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({
		QRhiVertexInputBinding(sizeof(float) * 2),
		QRhiVertexInputBinding(sizeof(float) * 16, QRhiVertexInputBinding::Classification::PerInstance),
		});
	inputLayout.setAttributes({
		QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
		QRhiVertexInputAttribute(1, 1, QRhiVertexInputAttribute::Float4, 0,0),
		QRhiVertexInputAttribute(1, 2, QRhiVertexInputAttribute::Float4, 4 * sizeof(float),1),
		QRhiVertexInputAttribute(1, 3, QRhiVertexInputAttribute::Float4, 8 * sizeof(float),2),
		QRhiVertexInputAttribute(1, 4, QRhiVertexInputAttribute::Float4, 12 * sizeof(float),3),
		});
	QString vsCode = R"(#version 440
		layout(location = 0) in vec2 inPosition;
		layout(location = 1) in mat4 inInstTransform;
		layout(location = 0) out vec4 vColor;
		layout(std140,binding = 0) uniform UniformBuffer{
			mat4 M;
			mat4 V;
			mat4 P;
			vec4 Color;
		}UBO;

		out gl_PerVertex{
			vec4 gl_Position;
		};
		%1
	)";
	if (bFacingCamera) {
		vsCode = vsCode.arg(R"(
			void main(){
				mat4 Transform    = UBO.M * inInstTransform;
				mat3 CorrRotation = inverse(mat3(UBO.V));
				vec3 CorrPos      = CorrRotation * vec3(inPosition,0.0f);
				gl_Position = UBO.P * UBO.V * Transform * vec4(CorrPos, 1.0); 
				vColor = UBO.Color;
			})");
	}
	else {
		vsCode = vsCode.arg(R"(
			void main(){
				mat4 Transform = UBO.V * UBO.M * inInstTransform;
				gl_Position =  UBO.P * Transform * vec4(inPosition,0.0f,1.0f);
				vColor = UBO.Color;
			})");
	}


	QShader vs = mRhi->newShaderFromCode(QShader::Stage::VertexStage, vsCode.toLocal8Bit());

	QShader fs = mRhi->newShaderFromCode(QShader::Stage::FragmentStage, QString(R"(#version 440
		layout(location = 0) in vec4 vColor;
		layout(location = 0) out vec4 BaseColor;
		layout(location = 1) out vec4 DebugId;
		void main(){
			BaseColor = vColor;
			%1;
		}
		)")
	#ifdef QENGINE_WITH_EDITOR	
		.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()) + ";")
	#else
		.arg("")
	#endif
		.toLocal8Bit()
	);

	mPipeline.reset(mRhi->newGraphicsPipeline());
	mPipeline->setVertexInputLayout(inputLayout);
	QVector<QRhiGraphicsPipeline::TargetBlend> targetBlends(getBasePass()->getRenderTargetColorAttachments().size());
	mPipeline->setTargetBlends(targetBlends.begin(), targetBlends.end());
	mPipeline->setTopology(QRhiGraphicsPipeline::Topology::TriangleStrip);
	mPipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	mPipeline->setDepthTest(true);
	mPipeline->setDepthWrite(true);
	mPipeline->setSampleCount(getBasePass()->getSampleCount());
	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		 QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, mUniformBuffer.get())
	});
	mBindings->create();
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(getBasePass()->getRenderPassDescriptor());
	mPipeline->create();
}

void QParticlesRenderComponent::onPreUpdate(QRhiCommandBuffer* cmdBuffer) {
	mEmitter->onTick(cmdBuffer);
	if (auto gpuEmitter = qobject_cast<QGpuParticleEmitter*>(mEmitter)) {
		QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		VkBuffer indirectDispatchBuffer = *(VkBuffer*)gpuEmitter->getCurrentIndirectDispatchBuffer()->nativeBuffer().objects[0];
		VkBuffer indirectDrawBuffer = *(VkBuffer*)mIndirectDrawBuffer->nativeBuffer().objects[0];
		QVulkanInstance* vkInstance = QVulkanDefaultInstance::instance();
		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = offsetof(IndirectDrawBuffer, instanceCount);
		bufferCopy.size = sizeof(int);
		vkInstance->deviceFunctions(vkHandles->dev)->vkCmdCopyBuffer(vkCmdBufferHandle->commandBuffer, indirectDispatchBuffer, indirectDrawBuffer, 1, &bufferCopy);
	}
}

void QParticlesRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), &ParticleShape);
	if (mIndirectDrawBuffer) {
		IndirectDrawBuffer indirectDraw;
		indirectDraw.vertexCount = 4;
		indirectDraw.instanceCount = 0;
		indirectDraw.firstVertex = 0;
		indirectDraw.firstInstance = 0;
		batch->uploadStaticBuffer(mIndirectDrawBuffer.get(), &indirectDraw);
	}
}

void QParticlesRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	mUniform.M = getModelMatrix().toGenericMatrix<4, 4>();
	mUniform.V = getViewMatrix().toGenericMatrix<4, 4>();
	mUniform.P = getProjectionMatrixWithCorr().toGenericMatrix<4, 4>();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &mUniform);
}

void QParticlesRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	if (auto cpuEmitter = qobject_cast<QCpuParticleEmitter*>(mEmitter)) {
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		QRhiCommandBuffer::VertexInput VertexInputs[] = {
			{mVertexBuffer.get(), 0},
			{mEmitter->getTransformBuffer(),0 }
		};
		cmdBuffer->setVertexInput(0, 2, VertexInputs);
		cmdBuffer->draw(4, cpuEmitter->getNumOfParticle());
	}
	else{
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		QRhiCommandBuffer::VertexInput VertexInputs[] = {
			{mVertexBuffer.get(), 0},
			{mEmitter->getTransformBuffer(),0 }
		};
		cmdBuffer->setVertexInput(0, 2, VertexInputs);
		QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		auto buffer = mIndirectDrawBuffer->nativeBuffer();
		VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
		QVulkanInstance* vkInstance = QVulkanDefaultInstance::instance();
		vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDrawIndirect(vkCmdBufferHandle->commandBuffer, vkBuffer, 0, 1, sizeof(IndirectDrawBuffer));
	}
}

bool QParticlesRenderComponent::isVaild() {
	return true;
}
