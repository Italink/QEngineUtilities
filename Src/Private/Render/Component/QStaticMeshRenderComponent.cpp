#include "Render/Component/QStaticMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/DebugUtils.h"

QStaticMeshRenderComponent::QStaticMeshRenderComponent() {
}

void QStaticMeshRenderComponent::setStaticMesh(QSharedPointer<QStaticMesh> val) {
	mStaticMesh = val;
	if (mStaticMesh) {
		sigonRebuildResource.request();
		sigonRebuildPipeline.request();
	}
}

void QStaticMeshRenderComponent::onRebuildResource() {
	if (mStaticMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticMesh::Vertex) * mStaticMesh->mVertices.size()));
	mVertexBuffer->create();
	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticMesh::Index) * mStaticMesh->mIndices.size()));
	mIndexBuffer->create();

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

		setupShaderForSubmesh(pipeline.get(), subMesh);
	}
}

void QStaticMeshRenderComponent::setupShaderForSubmesh(QRhiGraphicsPipelineBuilder* inPipeline, const QStaticMesh::SubMeshInfo& info) {
	inPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			layout(location = 0) out vec2 vUV;
			layout(location = 1) out vec3 vWorldPosition;
			layout(location = 2) out mat3 vTangentBasis;
			void main(){
				vUV = inUV;
				vWorldPosition = vec3(Transform.M * vec4(inPosition,1.0f));
				vTangentBasis =  mat3(transpose(inverse(Transform.M)))* mat3(inTangent, inBitangent, inNormal);
				gl_Position = Transform.MVP * vec4(inPosition,1.0f);
			}
			)");

	QColor BaseColor = info.materialProperties.value("BaseColor", QColor(255,255,255)).value<QColor>();
	bool bHasDiffuse = info.materialProperties.contains("Diffuse");
	if (bHasDiffuse) {
		inPipeline->addTexture(QRhiShaderStage::Fragment, QRhiGraphicsPipelineBuilder::TextureInfo::Texture2D, "Diffuse", info.materialProperties["Diffuse"].value<QImage>());
	}
	else {
		inPipeline->addUniformBlock(QRhiShaderStage::Fragment, "Material")
			->addParam<QColor>("BaseColor", BaseColor);
	}
	
	inPipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			BaseColor = %1;
			%2
			%3
			%4	
			%5;
		})").arg(bHasDiffuse ? "texture(Diffuse,vUV)" : "Material.BaseColor")
			.arg(getBasePass()->hasColorAttachment("Position") ? "Position = vec4(vWorldPosition  ,1); "  : "")
			.arg(getBasePass()->hasColorAttachment("Normal")   ? "Normal   = vec4(vTangentBasis[2],1);" : "")
			.arg(getBasePass()->hasColorAttachment("Tangent") ?  "Tangent  = vec4(vTangentBasis[0],1);" : "")
#ifdef QENGINE_WITH_EDITOR	
		.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()))
#else
		.arg("")
#endif
		.toLocal8Bit()
	);
}

void QStaticMeshRenderComponent::onRebuildPipeline() {
	for (auto& pipeline : mPipelines) {
		pipeline->create(this);
	}
}

void QStaticMeshRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mStaticMesh->mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mStaticMesh->mIndices.constData());
}

void QStaticMeshRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i].get();
		const QStaticMesh::SubMeshInfo& meshInfo = mStaticMesh->mSubmeshes[i];
		QMatrix4x4 MVP = calculateMatrixMVP() * meshInfo.localTransfrom;
		QMatrix4x4 M = calculateMatrixModel() * meshInfo.localTransfrom;
		pipeline->getUniformBlock("Transform")->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
		pipeline->getUniformBlock("Transform")->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
		pipeline->update(batch);
		if (pipeline->sigRebuild.receive()) {
			sigonRebuildPipeline.request();
		}
	}
}

void QStaticMeshRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i].get();
		const QStaticMesh::SubMeshInfo& meshInfo = mStaticMesh->mSubmeshes[i];
		cmdBuffer->setGraphicsPipeline(pipeline->getGraphicsPipeline());
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
