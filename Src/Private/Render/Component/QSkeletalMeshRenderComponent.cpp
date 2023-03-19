#include "Render/Component/QSkeletalMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/DebugUtils.h"

QSkeletalMeshRenderComponent::QSkeletalMeshRenderComponent(const QString& inSkeletalMeshPath) {
	if(!inSkeletalMeshPath.isEmpty())
		setupSkeletalMeshPath(inSkeletalMeshPath);
}

QString QSkeletalMeshRenderComponent::getSkeletalMeshPath() const {
	return mSkeletalMeshPath;
}

QSkeletalMeshRenderComponent* QSkeletalMeshRenderComponent::setupSkeletalMeshPath(QString inPath) {
	mSkeletalMeshPath = inPath;
	mSkeletalMesh = QSkeletalMesh::loadFromFile(inPath);
	sigonRebuildResource.request();
	sigonRebuildPipeline.request();
	return this;
}

void QSkeletalMeshRenderComponent::onRebuildResource() {
	if (mSkeletalMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QSkeletalMesh::Vertex) * mSkeletalMesh->mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QSkeletalMesh::Index) * mSkeletalMesh->mIndices.size()));
	mIndexBuffer->create();

	mUniformBlock = QSharedPointer<QRhiUniformBlock>::create(QRhiShaderStage::Vertex);
	mUniformBlock->setObjectName("Transform");
	mUniformBlock->addParam("MVP", MathUtils::Mat4())
		->addParam("M", MathUtils::Mat4())
		->addParam("Bone", mSkeletalMesh->mCurrentPosesMatrix);

	mUniformBlock->create(mRhi);
	for (auto& mesh : mSkeletalMesh->mSubmeshes) {
		QSharedPointer<QRhiGraphicsPipelineBuilder> mPipeline(new QRhiGraphicsPipelineBuilder());
		mPipelines << mPipeline;
		mPipeline->addUniformBlock(QRhiShaderStage::Vertex, mUniformBlock);
		mPipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QSkeletalMesh::Vertex))
		});
		mPipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QSkeletalMesh::Vertex,texCoord)),
			QRhiVertexInputAttributeEx("inBoneIndex",0, 5, QRhiVertexInputAttribute::UInt4, offsetof(QSkeletalMesh::Vertex,boneIndex)),
			QRhiVertexInputAttributeEx("inBoneWeight",0, 6, QRhiVertexInputAttribute::Float4, offsetof(QSkeletalMesh::Vertex,boneWeight))
			});

		mPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
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

		bool bHasDiffuse = mesh.materialProperties.contains("Diffuse");
		if (bHasDiffuse) {
			mPipeline->addTexture(QRhiShaderStage::Fragment, QRhiGraphicsPipelineBuilder::TextureInfo::Texture2D, "Diffuse", mesh.materialProperties["Diffuse"].value<QImage>());
		}
		mPipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			layout(location = 0) in vec2 vUV;
			layout(location = 1) in vec3 vWorldPosition;
			layout(location = 2) in mat3 vTangentBasis;
			void main(){
				BaseColor = %1;
				%2
			}
		)").arg(bHasDiffuse ? "texture(Diffuse,vUV)" : "vec4(vTangentBasis[2],1)")
#ifdef QENGINE_WITH_EDITOR	
			.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()) + ";")
#else
			.arg("")
#endif
			.toLocal8Bit()
		);
	}
}

void QSkeletalMeshRenderComponent::onRebuildPipeline() {
	for (auto mPipeline : mPipelines) {
		mPipeline->create(this);
	}
}

void QSkeletalMeshRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mSkeletalMesh->mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mSkeletalMesh->mIndices.constData());
}

void QSkeletalMeshRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	for (auto mPipeline : mPipelines) {
		QMatrix4x4 MVP = calculateMatrixMVP();
		QMatrix4x4 M = calculateMatrixModel();
		mPipeline->getUniformBlock("Transform")->setParamValue("MVP", MVP.toGenericMatrix<4,4>());
		mPipeline->getUniformBlock("Transform")->setParamValue("M", M.toGenericMatrix<4, 4>());
		mPipeline->getUniformBlock("Transform")->setParamValue("Bone", mSkeletalMesh->mCurrentPosesMatrix);
		mPipeline->update(batch);
		if (mPipeline->sigRebuild.receive()) {
			sigonRebuildPipeline.request();
		}
	}
}

void QSkeletalMeshRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* mPipeline = mPipelines[i].get();
		const QSkeletalMesh::SubMeshInfo& meshInfo = mSkeletalMesh->mSubmeshes[i];
		cmdBuffer->setGraphicsPipeline(mPipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), meshInfo.verticesOffset * sizeof(QSkeletalMesh::Vertex));
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), meshInfo.indicesOffset * sizeof(QSkeletalMesh::Index), QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(meshInfo.indicesRange);
	}
}

bool QSkeletalMeshRenderComponent::isVaild() {
	return !mSkeletalMesh.isNull();
}
