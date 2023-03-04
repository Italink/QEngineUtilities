#include "Render/Components/QStaticMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/DebugUtils.h"

QStaticMeshRenderComponent::QStaticMeshRenderComponent(const QString& inStaticMeshPath) {
	setupStaticMeshPath(inStaticMeshPath);
}

QString QStaticMeshRenderComponent::getStaticMeshPath() const {
	return mStaticMeshPath;
}

QStaticMeshRenderComponent* QStaticMeshRenderComponent::setupStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	mStaticMesh = QStaticMesh::loadFromFile(inPath);
	sigonRebuildResource.request();
	sigonRebuildPipeline.request();
	return this;
}

void QStaticMeshRenderComponent::onRebuildResource() {
	if (mStaticMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticMesh::Vertex) * mStaticMesh->mVertices.size()));
	mVertexBuffer->create();
	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticMesh::Index) * mStaticMesh->mIndices.size()));
	mIndexBuffer->create();

	for (auto& subMesh : mStaticMesh->mSubmeshes) {
		QSharedPointer<QRhiGraphicsPipelineBuilder> mPipeline(new QRhiGraphicsPipelineBuilder());
		mPipelines << mPipeline;
		mPipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addParam("MVP", QGenericMatrix<4, 4, float>())
			->addParam("M", QGenericMatrix<4, 4, float>());

		mPipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QStaticMesh::Vertex))
			});

		mPipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QStaticMesh::Vertex,texCoord))
			});

		mPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			layout(location = 0) out vec2 vUV;
			layout(location = 1) out vec3 vWorldPosition;
			layout(location = 2) out mat3 vTangentBasis;
			void main(){
				vUV = inUV;
				vWorldPosition = vec3(Transform.M * vec4(inPosition,1.0f));
				vTangentBasis =  mat3(Transform.M) * mat3(inTangent, inBitangent, inNormal);
				gl_Position = Transform.MVP * vec4(inPosition,1.0f);
			}
			)");
		bool bHasDiffuse = subMesh.materialInfo.contains("Diffuse");
		if (bHasDiffuse) {
			mPipeline->addTexture(QRhiShaderStage::Fragment, QRhiGraphicsPipelineBuilder::TextureInfo::Texture2D, "Diffuse", subMesh.materialInfo["Diffuse"]);
		}
		mPipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			BaseColor = %1;
			%2;
		})").arg(bHasDiffuse ? "texture(Diffuse,vUV)" : "vec4(vTangentBasis[2],1)")
		#ifdef QENGINE_WITH_EDITOR	
			.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()))
		#else
			.arg("")
		#endif
			.toLocal8Bit()
		);
	}
}

void QStaticMeshRenderComponent::onRebuildPipeline() {
	for (auto& mPipeline : mPipelines) {
		mPipeline->create(this);
	}
}

void QStaticMeshRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mStaticMesh->mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mStaticMesh->mIndices.constData());
}

void QStaticMeshRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* mPipeline = mPipelines[i].get();
		const QStaticMesh::SubMeshInfo& meshInfo = mStaticMesh->mSubmeshes[i];
		QMatrix4x4 MVP = calculateMatrixMVP() * meshInfo.localTransfrom;
		QMatrix4x4 M = calculateMatrixModel() * meshInfo.localTransfrom;
		mPipeline->getUniformBlock("Transform")->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
		mPipeline->getUniformBlock("Transform")->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
		mPipeline->update(batch);
		if (mPipeline->sigRebuild.receive()) {
			sigonRebuildPipeline.request();
		}
	}
}

void QStaticMeshRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* mPipeline = mPipelines[i].get();
		const QStaticMesh::SubMeshInfo& meshInfo = mStaticMesh->mSubmeshes[i];
		cmdBuffer->setGraphicsPipeline(mPipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), meshInfo.verticesOffset * sizeof(QStaticMesh::Vertex));
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), meshInfo.indicesOffset * sizeof(QStaticMesh::Index), QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(meshInfo.indicesRange);
	}
}

bool QStaticMeshRenderComponent::isVaild() {
	return !mStaticMesh.isNull();
}