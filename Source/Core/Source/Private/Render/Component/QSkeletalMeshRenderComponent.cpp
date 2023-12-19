#include "Render/Component/QSkeletalMeshRenderComponent.h"
#include "QEngineObjectManager.h"

QSkeletalMeshRenderComponent::QSkeletalMeshRenderComponent() {
}

void QSkeletalMeshRenderComponent::setSkeletalMesh(QSharedPointer<QSkeletalMesh> val) {
	mSkeletalMesh = val;
	if (mSkeletalMesh) {
		mMaterialGroup.reset(new QRhiMaterialGroup(mSkeletalMesh->mMaterials));
		mSigRebuildResource.request();
	}
}

void QSkeletalMeshRenderComponent::onRebuildResource() {
	if (mSkeletalMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QSkeletalMesh::Vertex) * mSkeletalMesh->mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QSkeletalMesh::Index) * mSkeletalMesh->mIndices.size()));
	mIndexBuffer->create();

	mUniformBlock = QSharedPointer<QRhiUniformBlock>::create();
	mUniformBlock->setObjectName("Transform");
	mUniformBlock->addParam("MVP", MathUtils::Mat4())
		->addParam("M", MathUtils::Mat4())
		->addParam("Bone", mSkeletalMesh->mCurrentPosesMatrix);

	mUniformBlock->create(mRhi);
	for (auto& mesh : mSkeletalMesh->mSubmeshes) {
		QSharedPointer<QPrimitiveRenderProxy> proxy = newPrimitiveRenderProxy();
		mPipelines << proxy;
		proxy->addUniformBlock(QRhiShaderStage::Vertex, mUniformBlock);
		proxy->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QSkeletalMesh::Vertex))
		});
		proxy->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QSkeletalMesh::Vertex,texCoord)),
			QRhiVertexInputAttributeEx("inBoneIndex",0, 5, QRhiVertexInputAttribute::UInt4, offsetof(QSkeletalMesh::Vertex,boneIndex)),
			QRhiVertexInputAttributeEx("inBoneWeight",0, 6, QRhiVertexInputAttribute::Float4, offsetof(QSkeletalMesh::Vertex,boneWeight))
		});
		proxy->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			layout(location = 0) out vec2 vUV;
			layout(location = 1) out vec3 vWorldPosition;
			layout(location = 2) out mat3 vTangentBasis;
			void main(){
				mat4 BoneTransform =  Transform.Bone[inBoneIndex[0]] * inBoneWeight[0];
						BoneTransform += Transform.Bone[inBoneIndex[1]] * inBoneWeight[1];
						BoneTransform += Transform.Bone[inBoneIndex[2]] * inBoneWeight[2];
						BoneTransform += Transform.Bone[inBoneIndex[3]] * inBoneWeight[3];
				vUV = inUV;
				vec4 pos = BoneTransform * vec4(inPosition,1.0f);
				vWorldPosition = vec3(Transform.M * pos);
				vTangentBasis =  mat3(Transform.M) * mat3(inTangent, inBitangent, inNormal);
				gl_Position = Transform.MVP * pos;
			}
		)");

		auto materialDesc = mMaterialGroup->getMaterialDesc(mesh.materialIndex);
		proxy->addMaterial(materialDesc);

		proxy->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			layout(location = 0) in vec2 vUV;
			layout(location = 1) in vec3 vWorldPosition;
			layout(location = 2) in mat3 vTangentBasis;
			void main(){
				%1;
				%2
				%3
				%4	
				%5
			})").arg(QString("BaseColor = %1;").arg(materialDesc->getOrCreateBaseColorExpression()))
				.arg(hasColorAttachment("Position") ? "Position = vec4(vWorldPosition  ,1);" : "")
				.arg(hasColorAttachment("Normal") ? QString("Normal    = vec4(normalize(vTangentBasis * %1 ),1.0f);").arg(materialDesc->getNormalExpression()) : "")
				.arg(hasColorAttachment("Metallic") ? QString("Metallic  = %1;").arg(materialDesc->getOrCreateMetallicExpression()) : "")
				.arg(hasColorAttachment("Roughness") ? QString("Roughness = %1;").arg(materialDesc->getOrCreateRoughnessExpression()) : "")
				.toLocal8Bit()
			);
		proxy->setOnUpload([this](QRhiResourceUpdateBatch* batch) {
			if (mVertexBuffer) {
				batch->uploadStaticBuffer(mVertexBuffer.get(), mSkeletalMesh->mVertices.constData());
				batch->uploadStaticBuffer(mIndexBuffer.get(), mSkeletalMesh->mIndices.constData());
			}
		});
		proxy->setOnUpdate([this](QRhiResourceUpdateBatch* batch, const QPrimitiveRenderProxy::UniformBlocks& blocks, const QPrimitiveRenderProxy::UpdateContext& ctx) {
			QMatrix4x4 M = getModelMatrix();
			QMatrix4x4 MVP = ctx.projectionMatrixWithCorr * ctx.viewMatrix * M;
			blocks["Transform"]->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
			blocks["Transform"]->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
			blocks["Transform"]->setParamValue("Bone", mSkeletalMesh->mCurrentPosesMatrix);
		});
		proxy->setOnDraw([this, mesh](QRhiCommandBuffer* cmdBuffer) {
			const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), mesh.verticesOffset * sizeof(QSkeletalMesh::Vertex));
			cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), mesh.indicesOffset * sizeof(QSkeletalMesh::Index), QRhiCommandBuffer::IndexUInt32);
			cmdBuffer->drawIndexed(mesh.indicesRange);
		});
	}
}

QENGINE_REGISTER_CLASS(QSkeletalMeshRenderComponent)