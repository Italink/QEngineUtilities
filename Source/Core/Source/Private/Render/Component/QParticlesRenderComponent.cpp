#include "Render/Component/QParticlesRenderComponent.h"
#include "QVulkanInstance"
#include "qvulkanfunctions.h"
#include "QEngineObjectManager.h"

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
}

void QParticlesRenderComponent::setParticleShape(QSharedPointer<QStaticMesh> inStaticMesh)
{
	mStaticMesh = inStaticMesh;
	mSigRebuildResource.request();
}

QSharedPointer<QStaticMesh> QParticlesRenderComponent::getParticleShape()
{
	return mStaticMesh;
}

void QParticlesRenderComponent::setFacingCamera(bool val) {
	bFacingCamera = val;
	mSigRebuildResource.request();
}

bool QParticlesRenderComponent::getFacingCamera() const {
	return bFacingCamera;
}

void QParticlesRenderComponent::onRebuildResource() {
	if (mStaticMesh.isNull()) {
		QImage image(10, 10, QImage::Format_RGBA8888);
		image.fill(Qt::white);
		mStaticMesh = QStaticMesh::CreateFromImage(image);
	}
	mEmitter->setupRhi(mRhi);
	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticMesh::Vertex) * mStaticMesh->mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticMesh::Index) * mStaticMesh->mIndices.size()));
	mIndexBuffer->create();

	mRenderProxy = newPrimitiveRenderProxy();
	mRenderProxy->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
		->addParam("M", QGenericMatrix<4, 4, float>())
		->addParam("V", QGenericMatrix<4, 4, float>())
		->addParam("P", QGenericMatrix<4, 4, float>());

	QVector<QRhiGraphicsPipeline::TargetBlend> blendState(getColorAttachmentCount());
	for (auto& state : blendState) {
		state.enable = true;
		state.srcColor = QRhiGraphicsPipeline::SrcAlpha;
		state.dstColor = QRhiGraphicsPipeline::OneMinusSrcAlpha;
	}
	mRenderProxy->setBlendStates(blendState);
	mRenderProxy->setDepthTest(false);
	mRenderProxy->setInputBindings({
		QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QStaticMesh::Vertex)),
		QRhiVertexInputBindingEx(mIndirectDrawBuffer.get(),sizeof(float) * 16, 0 , QRhiVertexInputBinding::Classification::PerInstance)
		});

	mRenderProxy->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QStaticMesh::Vertex,uv)),

			QRhiVertexInputAttributeEx("inInstTransform", 1, 5, QRhiVertexInputAttribute::Float4, 0,0),
			QRhiVertexInputAttributeEx("inInstTransform", 1, 6, QRhiVertexInputAttribute::Float4, 4 * sizeof(float),1),
			QRhiVertexInputAttributeEx("inInstTransform", 1, 7, QRhiVertexInputAttribute::Float4, 8 * sizeof(float),2),
			QRhiVertexInputAttributeEx("inInstTransform", 1, 8, QRhiVertexInputAttribute::Float4, 12 * sizeof(float),3),
		});

	if (bFacingCamera) {
		mRenderProxy->setShaderMainCode(QRhiShaderStage::Vertex, R"(
				layout(location = 0) out vec2 vUV;
				layout(location = 1) out vec3 vWorldPosition;
				layout(location = 2) out mat3 vTangentBasis;
				void main(){
					mat4 Trans    = Transform.M * inInstTransform;
					mat3 CorrRotation = inverse(mat3(Transform.V));
					vec3 CorrPos      = CorrRotation * vec3(inPosition);
					gl_Position = Transform.P * Transform.V * Trans * vec4(CorrPos, 1.0); 
					vUV = inUV;
					vWorldPosition = vec3(Transform.M * vec4(inPosition,1.0f));
					vTangentBasis = mat3(Transform.M) * mat3(inTangent, inBitangent, inNormal);
				}
		)");
	}
	else {
		mRenderProxy->setShaderMainCode(QRhiShaderStage::Vertex, R"(
				layout(location = 0) out vec2 vUV;
				layout(location = 1) out vec3 vWorldPosition;
				layout(location = 2) out mat3 vTangentBasis;
				void main(){
					mat4 model = Transform.M *inInstTransform;
					gl_Position =  Transform.P * Transform.V * model * vec4(inPosition,1.0f);
					vUV = inUV;
					vWorldPosition = vec3(model * vec4(inPosition,1.0f));
					vTangentBasis = mat3(model) * mat3(inTangent, inBitangent, inNormal);
				}
		)");
	}

	mMaterialGroup.reset(new QRhiMaterialGroup(mStaticMesh->mMaterials));
	auto materialDesc = mMaterialGroup->getMaterialDesc(mStaticMesh->mSubmeshes[0].materialIndex);
	mRenderProxy->addMaterial(materialDesc);
	mRenderProxy->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			layout(location = 0) in vec2 vUV;
			layout(location = 1) in vec3 vWorldPosition;
			layout(location = 2) in mat3 vTangentBasis;
			void main(){
				%1
				%2
				%3
				%4	
				%5
				%6
			})")
		.arg(QString("BaseColor = %1;").arg(materialDesc->getOrCreateBaseColorExpression()))
		.arg(hasColorAttachment("Position") ? "Position = vec4(vWorldPosition  ,1);" : "")
		.arg(hasColorAttachment("Normal") ? QString("Normal    = vec4(normalize(vTangentBasis * %1 ),1.0f);").arg(materialDesc->getNormalExpression()) : "")
		.arg(hasColorAttachment("Specular") ? QString("Specular  = %1;").arg(materialDesc->getOrCreateSpecularExpression()) : "")
		.arg(hasColorAttachment("Metallic") ? QString("Metallic  = %1;").arg(materialDesc->getOrCreateMetallicExpression()) : "")
		.arg(hasColorAttachment("Roughness") ? QString("Roughness = %1;").arg(materialDesc->getOrCreateRoughnessExpression()) : "")
		.toLocal8Bit()
	);
	mRenderProxy->setOnUpload([this](QRhiResourceUpdateBatch* batch) {
		if (mStaticMesh.isNull())
			return;

		if (mVertexBuffer) {
			batch->uploadStaticBuffer(mVertexBuffer.get(), mStaticMesh->mVertices.constData());
			batch->uploadStaticBuffer(mIndexBuffer.get(), mStaticMesh->mIndices.constData());
		}

		if (mIndirectDrawBuffer) {
			IndirectDrawBuffer indirectDraw;
			indirectDraw.indexCount = mStaticMesh->mSubmeshes[0].indicesRange;
			indirectDraw.instanceCount = 0;
			indirectDraw.firstIndex = 0;
			indirectDraw.vertexOffset = 0;
			indirectDraw.firstInstance = 0;
			batch->uploadStaticBuffer(mIndirectDrawBuffer.get(), &indirectDraw);
		}
	});
	mRenderProxy->setOnUpdate([this](QRhiResourceUpdateBatch* batch, const QPrimitiveRenderProxy::UniformBlocks& blocks, const QPrimitiveRenderProxy::UpdateContext& ctx) {
		QMatrix4x4 M = getModelMatrix();
		blocks["Transform"]->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
		blocks["Transform"]->setParamValue("V", QVariant::fromValue(ctx.viewMatrix.toGenericMatrix<4, 4>()));
		blocks["Transform"]->setParamValue("P", QVariant::fromValue(ctx.projectionMatrixWithCorr.toGenericMatrix<4, 4>()));

	});
	mRenderProxy->setOnDraw([this](QRhiCommandBuffer* cmdBuffer) {
		if (auto cpuEmitter = qobject_cast<QCpuParticleEmitter*>(mEmitter)) {
			QRhiCommandBuffer::VertexInput VertexInputs[] = {
				{mVertexBuffer.get(), 0},
				{mEmitter->getTransformBuffer(),0 }
			};
			cmdBuffer->setVertexInput(0, 2, VertexInputs, mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexFormat::IndexUInt32);
			cmdBuffer->drawIndexed(mStaticMesh->mSubmeshes[0].indicesRange, cpuEmitter->getNumOfParticle());
		}
		else {
			QRhiCommandBuffer::VertexInput VertexInputs[] = {
				{mVertexBuffer.get(), 0},
				{mEmitter->getTransformBuffer(),0 }
			};
			cmdBuffer->setVertexInput(0, 2, VertexInputs, mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexUInt32);
			QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
			QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
			auto buffer = mIndirectDrawBuffer->nativeBuffer();
			VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
			QVulkanInstance* vkInstance = (*(QRhiVulkan**)(mRhi))->inst;
			vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDrawIndexedIndirect(vkCmdBufferHandle->commandBuffer, vkBuffer, 0, 1, sizeof(IndirectDrawBuffer));
		}
	});

	if (mEmitter && mEmitter->metaObject()->inherits(&QGpuParticleEmitter::staticMetaObject)) {
		mIndirectDrawBuffer.reset(QRhiHelper::newVkBuffer(mRhi, QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, sizeof(IndirectDrawBuffer)));
		mIndirectDrawBuffer->create();
		mIndirectDrawBuffer->setName("IndirectDrawBuffer");
	}
}

void QParticlesRenderComponent::onPreRenderTick(QRhiCommandBuffer* cmdBuffer)
{
	mEmitter->onTick(cmdBuffer);
	if (auto gpuEmitter = qobject_cast<QGpuParticleEmitter*>(mEmitter)) {
		QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		VkBuffer indirectDispatchBuffer = *(VkBuffer*)gpuEmitter->getCurrentIndirectDispatchBuffer()->nativeBuffer().objects[0];
		VkBuffer indirectDrawBuffer = *(VkBuffer*)mIndirectDrawBuffer->nativeBuffer().objects[0];
		QVulkanInstance* vkInstance = (*(QRhiVulkan**)(mRhi))->inst;
		VkBufferCopy bufferCopy;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = offsetof(IndirectDrawBuffer, instanceCount);
		bufferCopy.size = sizeof(int);
		vkInstance->deviceFunctions(vkHandles->dev)->vkCmdCopyBuffer(vkCmdBufferHandle->commandBuffer, indirectDispatchBuffer, indirectDrawBuffer, 1, &bufferCopy);
	}
}

QENGINE_REGISTER_CLASS(QParticlesRenderComponent)