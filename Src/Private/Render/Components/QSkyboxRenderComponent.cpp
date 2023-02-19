#include "Render/Components/QSkyboxRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/DebugUtils.h"

static float CubeData[] = { // Y up, front = CCW
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
};

QImage QSkyboxRenderComponent::getSkyBoxImage() const {
	return mSkyBoxImage;
}

QSkyboxRenderComponent::QSkyboxRenderComponent()
{
	setScale3D(QVector3D(2000, 2000, 2000));
}

QSkyboxRenderComponent* QSkyboxRenderComponent::setupSkyBoxImage(QImage inImage) {
	mSkyBoxImage = inImage.convertToFormat(QImage::Format::Format_RGBA8888);
	sigonRebuildResource.request();
	sigonRebuildPipeline.request();
	return this;
}

void QSkyboxRenderComponent::onRebuildResource() {
	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(CubeData)));
	mVertexBuffer->create();

	mPipeline.reset(new QRhiGraphicsPipelineBuilder);
	mPipeline->setDepthWrite(false);
	mPipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
		->addParam("MVP", QGenericMatrix<4, 4, float>());


	mPipeline->setInputBindings({
		QRhiVertexInputBindingEx(mVertexBuffer.get(), sizeof(QVector3D) )
	});

	mPipeline->setInputAttribute({
		QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3,0),
	});

	mPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
		layout(location = 0) out vec3 vPosition;
		void main(){
			vPosition = inPosition;
			gl_Position = Transform.MVP * vec4(inPosition,1.0f);
			gl_Position.z = 1.0f;
		}
	)");
	mPipeline->addTexture(QRhiShaderStage::Fragment, QRhiGraphicsPipelineBuilder::TextureInfo::Cube, "Skybox", mSkyBoxImage);
	mPipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
		layout(location = 0) in vec3 vPosition;
		void main(){
			BaseColor = texture(Skybox,vPosition);
			DebugId = %1;
		}
		)").arg(DebugUtils::convertIdToVec4Code(getID()))
		.toLocal8Bit()
	);
}

void QSkyboxRenderComponent::onRebuildPipeline() {
	mPipeline->create(this);
}

void QSkyboxRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), CubeData);
}

void QSkyboxRenderComponent::onPreUpdate(QRhiCommandBuffer* cmdBuffer) {

}

void QSkyboxRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	QMatrix4x4 MVP = calculateMatrixMVP();
	mPipeline->getUniformBlock("Transform")->setParamValue("MVP", MVP.toGenericMatrix<4, 4>());
	mPipeline->update(batch);
	if (mPipeline->sigRebuild.receive()) {
		sigonRebuildPipeline.request();
	}
}

void QSkyboxRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	const QRhiCommandBuffer::VertexInput VertexInput(mVertexBuffer.get(), 0);

	cmdBuffer->setGraphicsPipeline(mPipeline->getGraphicsPipeline());
	cmdBuffer->setViewport(viewport);
	cmdBuffer->setShaderResources();
	cmdBuffer->setVertexInput(0, 1, &VertexInput, mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexUInt32);
	cmdBuffer->draw(36);
}

bool QSkyboxRenderComponent::isVaild() {
	return !mSkyBoxImage.isNull();
}
