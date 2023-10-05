#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Utils/DebugUtils.h"
#include "Utils/MathUtils.h"

QStaticMeshRenderComponent::QStaticMeshRenderComponent() {
}

void QStaticMeshRenderComponent::setStaticMesh(QSharedPointer<QStaticMesh> val) {
	mStaticMesh = val;
	if (mStaticMesh) {
		mMaterialGroup.reset(new QRhiMaterialGroup(mStaticMesh->mMaterials));
		mSigRebuildResource.request();
	}
}

void QStaticMeshRenderComponent::onRebuildResource() {
	if (mStaticMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticMesh::Vertex) * mStaticMesh->mVertices.size()));
	mVertexBuffer->create();
	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticMesh::Index) * mStaticMesh->mIndices.size()));
	mIndexBuffer->create();

	mProxies.clear();
	for (auto& subMesh : mStaticMesh->mSubmeshes) {
		QSharedPointer<QPrimitiveRenderProxy> proxy = newPrimitiveRenderProxy();
		mProxies << proxy;

		proxy->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addParam("MVP", QGenericMatrix<4, 4, float>())
			->addParam("M", QGenericMatrix<4, 4, float>());

		proxy->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QStaticMesh::Vertex))
		});

		proxy->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QStaticMesh::Vertex,uv))
		});

		proxy->setShaderMainCode(QRhiShaderStage::Vertex, R"(
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
		proxy->addMaterial(materialDesc);

		proxy->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
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
			.arg(hasColorAttachment("Position")	? "Position = vec4(vWorldPosition  ,1);" : "")
			.arg(hasColorAttachment("Normal")	? QString("Normal    = vec4(normalize(vTangentBasis * %1 ),1.0f);").arg(materialDesc->getNormalExpression()) : "")
			.arg(hasColorAttachment("Specular")	? QString("Specular  = %1;").arg(materialDesc->getOrCreateSpecularExpression()) : "")
			.arg(hasColorAttachment("Metallic")	? QString("Metallic  = %1;").arg(materialDesc->getOrCreateMetallicExpression()) : "")
			.arg(hasColorAttachment("Roughness") ? QString("Roughness = %1;").arg(materialDesc->getOrCreateRoughnessExpression()) : "")
			.toLocal8Bit()
		);

		proxy->setOnUpload([this](QRhiResourceUpdateBatch* batch) {
			if (mVertexBuffer) {
				batch->uploadStaticBuffer(mVertexBuffer.get(), mStaticMesh->mVertices.constData());
				batch->uploadStaticBuffer(mIndexBuffer.get(), mStaticMesh->mIndices.constData());
			}
		});

		proxy->setOnUpdate([this, subMesh](QRhiResourceUpdateBatch* batch, const QPrimitiveRenderProxy::UniformBlocks& blocks, const QPrimitiveRenderProxy::UpdateContext& ctx) {
			QMatrix4x4 M = getModelMatrix() * subMesh.localTransfrom;
			QMatrix4x4 MVP = ctx.projectionMatrixWithCorr * ctx.viewMatrix * M;
			blocks["Transform"]->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
			blocks["Transform"]->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
		});

		proxy->setOnDraw([this, subMesh](QRhiCommandBuffer* cmdBuffer) {
			const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), subMesh.verticesOffset * sizeof(QStaticMesh::Vertex));
			cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), subMesh.indicesOffset * sizeof(QStaticMesh::Index), QRhiCommandBuffer::IndexUInt32);
			cmdBuffer->drawIndexed(subMesh.indicesRange);
		});
	}
}

QSharedPointer<QStaticMesh> QStaticMeshRenderComponent::getStaticMesh() const
{
	return mStaticMesh;
}

QRhiMaterialGroup* QStaticMeshRenderComponent::getMaterialGroup()
{
	return mMaterialGroup.get();
}
