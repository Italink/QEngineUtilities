#include "Render/Component/QDynamicMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/DebugUtils.h"
#include "Type/QColor4D.h"

QDynamicMeshRenderComponent::QDynamicMeshRenderComponent() {

}

void QDynamicMeshRenderComponent::onRebuildResource() {
	mMaterialGroup.reset(new QRhiMaterialGroup(QSharedPointer<QMaterial>::create()));
	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::VertexBuffer, sizeof(Vertex) * 1));
	mVertexBuffer->create();

	mPipeline.reset(new QRhiGraphicsPipelineBuilder());
	mPipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
		->addParam("MVP", QGenericMatrix<4, 4, float>())
		->addParam("M", QGenericMatrix<4, 4, float>());

	mPipeline->setInputBindings({
		QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(Vertex))
	});

	mPipeline->setInputAttribute({
		QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(Vertex,position)),
		QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(Vertex,normal)),
		QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(Vertex,tangent)),
		QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(Vertex,bitangent)),
		QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(Vertex,texCoord))
	});

	mPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
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
	auto materialDesc = mMaterialGroup->getMaterialDesc(0);
	mPipeline->addMaterial(materialDesc);
	mPipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
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
		.arg(getBasePass()->hasColorAttachment("Position") ? "Position = vec4(vWorldPosition  ,1);" : "")
		.arg(getBasePass()->hasColorAttachment("Normal") ? QString("Normal    = vec4(normalize(vTangentBasis * %1 ),1.0f);").arg(materialDesc->getNormalExpression()) : "")
		.arg(getBasePass()->hasColorAttachment("Specular") ? QString("Specular  = %1;").arg(materialDesc->getOrCreateSpecularExpression()) : "")
		.arg(getBasePass()->hasColorAttachment("Metallic") ? QString("Metallic  = %1;").arg(materialDesc->getOrCreateMetallicExpression()) : "")
		.arg(getBasePass()->hasColorAttachment("Roughness") ? QString("Roughness = %1;").arg(materialDesc->getOrCreateRoughnessExpression()) : "")
#ifdef QENGINE_WITH_EDITOR	
		.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()) + ";")
#else
		.arg("")
#endif
		.toLocal8Bit()
	);
}

void QDynamicMeshRenderComponent::onRebuildPipeline() {
	mPipeline->create(this);
}

void QDynamicMeshRenderComponent::onUpload(QRhiResourceUpdateBatch*) {

}

void QDynamicMeshRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	onUpdateVertices(mVertices);
	if (!mVertices.isEmpty()) {
		if (mVertices.size() * sizeof(Vertex) != mVertexBuffer->size() ) {
			mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::VertexBuffer, sizeof(Vertex) * mVertices.size()));
			mVertexBuffer->create();
		}
		batch->updateDynamicBuffer(mVertexBuffer.get(), 0, sizeof(Vertex) * mVertices.size(), mVertices.data());
	}
	QMatrix4x4 MVP = calculateMatrixMVP();
	QMatrix4x4 M = calculateMatrixModel();
	mPipeline->getUniformBlock("Transform")->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
	mPipeline->getUniformBlock("Transform")->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
	mPipeline->update(batch);
	if (mPipeline->sigRebuild.receive()) {
		sigonRebuildPipeline.request();
	}
}

void QDynamicMeshRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	cmdBuffer->setGraphicsPipeline(mPipeline->getGraphicsPipeline());
	cmdBuffer->setViewport(viewport);
	cmdBuffer->setShaderResources();
	const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(),0);
	cmdBuffer->setVertexInput(0, 1, &vertexBindings);
	cmdBuffer->draw(mVertices.size());
}

bool QDynamicMeshRenderComponent::isVaild() {
	return true;
}
