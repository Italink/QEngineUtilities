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

void QStaticMeshRenderComponent::setupShaderForSubmesh(QRhiGraphicsPipelineBuilder* inPipeline, QStaticMesh::SubMeshInfo info) {
	inPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			layout(location = 0) out vec2 vUV;
			layout(location = 1) out vec3 vWorldPosition;
			layout(location = 2) out mat3 vTangentBasis;
			void main(){
				vUV = inUV;
				vWorldPosition = vec3(Transform.M * vec4(inPosition,1.0f));
				vec3 T = normalize(vec3(Transform.M * vec4(inTangent,   0.0)));
				vec3 B = normalize(vec3(Transform.M * vec4(inBitangent, 0.0)));
				vec3 N = normalize(vec3(Transform.M * vec4(inNormal,    0.0)));
				vTangentBasis = mat3(T, B, N);
				gl_Position = Transform.MVP * vec4(inPosition,1.0f);
			}
	)");

	bool bHasBaseColor = info.materialProperties.contains("BaseColor") || info.materialProperties.contains("Diffuse");
	if (!bHasBaseColor) {
		info.materialProperties["BaseColor"] = QColor(255, 255, 255, 255);
	}
	bool bHasNormal = info.materialProperties.contains("Normal");

	bool bHasMetalness = info.materialProperties.contains("Metalness");
	if (!bHasMetalness) {
		info.materialProperties["Metalness"] = 0.5f;;
	}
	bool bHasRoughness = info.materialProperties.contains("Roughness");
	if (!bHasRoughness) {
		info.materialProperties["Roughness"] = 1.0f;
	}
	inPipeline->addMaterialProperty(info.materialProperties);


	QByteArray NormalAssign = "vec4(vTangentBasis[2], 1)";
	if (getBasePass()->hasColorAttachment("Normal")) {
		if (info.materialProperties["Normal"].metaType() == QMetaType::fromType<QImage>()) 
			NormalAssign = R"(vec3 n = 2.0 * texture(uNormal, vUV).rgb - 1.0;			
							 Normal = vec4((normalize( vTangentBasis * n ) + 1.0) * 0.5,1.0f);)";
		else 
			NormalAssign = "Normal = vec4((normalize(vTangentBasis[2]) + 1.0) * 0.5,1.0f);";
	}

	QByteArray BaseColorAssign;
	if (info.materialProperties["BaseColor"].metaType() == QMetaType::fromType<QImage>()) {
		BaseColorAssign = "BaseColor = texture(uBaseColor,vUV);";
	}
	else if (info.materialProperties["Diffuse"].metaType() == QMetaType::fromType<QImage>()) {
		BaseColorAssign = "BaseColor = texture(uDiffuse,vUV);";
	}
	else {
		BaseColorAssign = "BaseColor = Material.BaseColor;";
	}

	QByteArray MetalnessAssign;
	if (getBasePass()->hasColorAttachment("Metalness")) {
		if (info.materialProperties["Metalness"].metaType() == QMetaType::fromType<QImage>()) 
			MetalnessAssign = "Metalness = texture(uMetalness,vUV).r;";
		else 
			MetalnessAssign = "Metalness = Material.Metalness;";
	}

	QByteArray RoughnessAssign;
	if (getBasePass()->hasColorAttachment("Metalness")) {
		if (info.materialProperties["Roughness"].metaType() == QMetaType::fromType<QImage>()) 
			RoughnessAssign = "Roughness = texture(uRoughness,vUV).r;";
		else 
			RoughnessAssign = "Roughness = Material.Roughness;";		
	}

	inPipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			%1;
			%2
			%3
			%4	
			%5
			%6
			%7
		})").arg(BaseColorAssign)
			.arg(getBasePass()->hasColorAttachment("Position") ? "Position = vec4(vWorldPosition  ,1);" : "")
			.arg(NormalAssign)
			.arg(getBasePass()->hasColorAttachment("Tangent") ?  "Tangent  = vec4((normalize(vTangentBasis[0]) + 1.0) * 0.5,1.0f);" : "")
			.arg(MetalnessAssign)
			.arg(RoughnessAssign)
#ifdef QENGINE_WITH_EDITOR	
		.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()) + ";")
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
