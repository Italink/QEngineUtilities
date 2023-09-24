#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/DebugUtils.h"
#include "Utils/MathUtils.h"

QStaticMeshRenderComponent::QStaticMeshRenderComponent() {
}

void QStaticMeshRenderComponent::setStaticMesh(QSharedPointer<QStaticMesh> val) {
	mStaticMesh = val;
	if (mStaticMesh) {
		mMaterialGroup.reset(new QRhiMaterialGroup(mStaticMesh->mMaterials));
		mSigRebuildResource.request();
		mSigRebuildPipeline.request();
	}
}

void QStaticMeshRenderComponent::onRebuildResource() {
	if (mStaticMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticMesh::Vertex) * mStaticMesh->mVertices.size()));
	mVertexBuffer->create();
	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticMesh::Index) * mStaticMesh->mIndices.size()));
	mIndexBuffer->create();

	mPipelines.clear();
	for (auto& subMesh : mStaticMesh->mSubmeshes) {
		QSharedPointer<QRhiGraphicsPipelineBuilder> pipeline(new QRhiGraphicsPipelineBuilder());
		mPipelines << pipeline;
		pipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addParam("MVP", QGenericMatrix<4, 4, float>())
			->addParam("M", QGenericMatrix<4, 4, float>());

		pipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QStaticMesh::Vertex))
		});

		pipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QStaticMesh::Vertex,uv))
		});

		pipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
				layout(location = 0) out vec2 vUV;
				layout(location = 1) out vec3 vWorldPosition;
				layout(location = 2) out mat3 vTangentBasis;
				void main(){
					gl_Position = Transform.MVP * vec4(inPosition,1.0f);
					vUV = inUV;
					vWorldPosition = vec3(Transform.M * vec4(inPosition,1.0f));
					vTangentBasis = mat3(Transform.M) * mat3(inTangent, inBitangent, inNormal);
				}
		)");

		auto materialDesc = mMaterialGroup->getMaterialDesc(subMesh.materialIndex);
		pipeline->addMaterial(materialDesc);

		pipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
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
				%7
			})")
			.arg(QString("BaseColor = %1;").arg(materialDesc->getOrCreateBaseColorExpression()))
			.arg(hasColorAttachment("Position")	? "Position = vec4(vWorldPosition  ,1);" : "")
			.arg(hasColorAttachment("Normal")	? QString("Normal    = vec4(normalize(vTangentBasis * %1 ),1.0f);").arg(materialDesc->getNormalExpression()) : "")
			.arg(hasColorAttachment("Specular")	? QString("Specular  = %1;").arg(materialDesc->getOrCreateSpecularExpression()) : "")
			.arg(hasColorAttachment("Metallic")	? QString("Metallic  = %1;").arg(materialDesc->getOrCreateMetallicExpression()) : "")
			.arg(hasColorAttachment("Roughness") ? QString("Roughness = %1;").arg(materialDesc->getOrCreateRoughnessExpression()) : "")
#ifdef QENGINE_WITH_EDITOR	
			.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()) + ";")
#else
			.arg("")
#endif
			.toLocal8Bit()
		);
	}
}

QSharedPointer<QStaticMesh> QStaticMeshRenderComponent::getStaticMesh() const
{
	return mStaticMesh;
}

void QStaticMeshRenderComponent::onRebuildPipeline() {
	for (auto& pipeline : mPipelines) {
		pipeline->create(this);
	}
}

QRhiMaterialGroup* QStaticMeshRenderComponent::getMaterialGroup()
{
	return mMaterialGroup.get();
}

void QStaticMeshRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	if (mVertexBuffer) {
		batch->uploadStaticBuffer(mVertexBuffer.get(), mStaticMesh->mVertices.constData());
		batch->uploadStaticBuffer(mIndexBuffer.get(), mStaticMesh->mIndices.constData());
	}
}

void QStaticMeshRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i].get();
		const QStaticMesh::SubMeshData& meshInfo = mStaticMesh->mSubmeshes[i];
		QMatrix4x4 M = getModelMatrix() * meshInfo.localTransfrom;
		QMatrix4x4 MVP = getProjectionMatrixWithCorr() * getViewMatrix() * M;
		pipeline->getUniformBlock("Transform")->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
		pipeline->getUniformBlock("Transform")->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
		pipeline->update(batch);
		if (pipeline->sigRebuild.ensure()) {
			mSigRebuildPipeline.request();
		}
	}
}

void QStaticMeshRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i].get();
		const QStaticMesh::SubMeshData& meshInfo = mStaticMesh->mSubmeshes[i];
		cmdBuffer->setGraphicsPipeline(pipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), meshInfo.verticesOffset * sizeof(QStaticMesh::Vertex));
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), meshInfo.indicesOffset * sizeof(QStaticMesh::Index), QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(meshInfo.indicesRange);
	}
}
