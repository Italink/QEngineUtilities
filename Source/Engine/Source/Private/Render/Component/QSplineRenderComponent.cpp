#include "Render/Component/QSplineRenderComponent.h"
#include "Utils/DebugUtils.h"
#include "Utils/MathUtils.h"

QSplineRenderComponent::QSplineRenderComponent() {

}

float QSplineRenderComponent::getLineWidth() const {
	if (mPipelineBuilder && mPipelineBuilder->getUniformBlock("UBO")) {
		return mPipelineBuilder->getUniformBlock("UBO")->getParamDesc("Width")->mValue.toFloat();
	}
	return 0.0f;
}

void QSplineRenderComponent::setLineWidth(float val) {
	if (mPipelineBuilder && mPipelineBuilder->getUniformBlock("UBO")) {
		mPipelineBuilder->getUniformBlock("UBO")->setParamValue("Width",val);
	}
}

QList<QSplinePoint> QSplineRenderComponent::getPoints() const {
	return mPoints;
}

void QSplineRenderComponent::setPoints(QList<QSplinePoint> val) {
	mPoints = val;
	mSigRebuildResource.request();
}

int QSplineRenderComponent::getSubdivisionAmount() const {
	return mSubdivisionAmount;
}

void QSplineRenderComponent::setSubdivisionAmount(int val) {
	mSubdivisionAmount = val;
	mSigRebuildResource.request();
}

void QSplineRenderComponent::onRebuildResource() {
	mSegmentData = {
		{0, -0.5, 0, 0} ,
		{0, -0.5, 1, 0},
		{0,  0.5, 1, 0},
		{0, -0.5, 0, 0},
		{0,  0.5, 1, 0},
		{0,  0.5, 0, 0}
	};

	for (int step = 0; step < mSubdivisionAmount; step++) {
		const float theta0 = M_PI / 2 + ((step + 0) * M_PI) / mSubdivisionAmount;
		const float theta1 = M_PI / 2 + ((step + 1) * M_PI) / mSubdivisionAmount;
		mSegmentData << QVector4D{ 0, 0, 0, 1 };
		mSegmentData.push_back({
			0.5f * qCos(theta0),
			0.5f * qSin(theta0),
			0.0f,
			1.0f
		});
		mSegmentData.push_back({
			0.5f * qCos(theta1),
			0.5f * qSin(theta1),
			0.0f,
			1.0f
		});
	}
	for (int step = 0; step < mSubdivisionAmount; step++) {
		const float theta0 = 3 * M_PI / 2 + ((step + 0) * M_PI) / mSubdivisionAmount;
		const float theta1 = 3 * M_PI / 2 + ((step + 1) * M_PI) / mSubdivisionAmount;
		mSegmentData << QVector4D{ 0, 0, 1, 1 };
		mSegmentData.push_back({
			0.5f * qCos(theta0),
			0.5f * qSin(theta0),
			1.0f,
			1.0f
		});
		mSegmentData.push_back({
			0.5f * qCos(theta1),
			0.5f * qSin(theta1),
			1.0f,
			1.0f
		});
	}

	mInstanceBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, sizeof(QSplinePoint)*mPoints.size()));
	mInstanceBuffer->create();

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(QVector4D) * mSegmentData.size()));
	mVertexBuffer->create();

	if (!mPipelineBuilder) {
		mPipelineBuilder.reset(new QRhiGraphicsPipelineBuilder);
		mPipelineBuilder->setTopology(QRhiGraphicsPipeline::Topology::Triangles);
		mPipelineBuilder->addUniformBlock(QRhiShaderStage::Vertex, "UBO")
			->addParam("MVP", QGenericMatrix<4, 4, float>())
			->addParam("Width", 5.0f)
			->addParam("ScreenResolution", QVector2D(800, 600));

		mPipelineBuilder->setInputBindings({
			QRhiVertexInputBindingEx(mInstanceBuffer.get(),sizeof(QSplinePoint),0 , QRhiVertexInputBinding::PerInstance) ,
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QVector4D)),
		});

		mPipelineBuilder->setInputAttribute({
			QRhiVertexInputAttributeEx("inPointA"		 ,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QSplinePoint,mPoint)),
			QRhiVertexInputAttributeEx("inColorA"		 ,0, 1, QRhiVertexInputAttribute::Float4, offsetof(QSplinePoint,mColor)),
			QRhiVertexInputAttributeEx("inPointB"		 ,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QSplinePoint,mPoint) + sizeof(QSplinePoint)),
			QRhiVertexInputAttributeEx("inColorB"		 ,0, 3, QRhiVertexInputAttribute::Float4, offsetof(QSplinePoint,mColor) + sizeof(QSplinePoint)),
			QRhiVertexInputAttributeEx("inPos" ,1, 4, QRhiVertexInputAttribute::Float3, 0),
		});

		mPipelineBuilder->setShaderMainCode(QRhiShaderStage::Vertex, R"(
				layout(location = 0) out vec4 vColor;
				void main(){
					vec4 clip0 = UBO.MVP * vec4(inPointA, 1.0);
					vec4 clip1 = UBO.MVP * vec4(inPointB, 1.0);
					vec2 screen0 = UBO.ScreenResolution * (0.5 * clip0.xy/clip0.w + 0.5);
					vec2 screen1 = UBO.ScreenResolution * (0.5 * clip1.xy/clip1.w + 0.5);
					vec2 xBasis = normalize(screen1 - screen0);
					vec2 yBasis = vec2(-xBasis.y, xBasis.x);
					vec2 pt0 = screen0 + UBO.Width * (inPos.x * xBasis + inPos.y * yBasis);
					vec2 pt1 = screen1 + UBO.Width * (inPos.x * xBasis + inPos.y * yBasis);
					vec2 pt = mix(pt0, pt1, inPos.z);
					vec4 clip = mix(clip0, clip1, inPos.z);
					gl_Position = vec4(clip.w * ((2.0 * pt) / UBO.ScreenResolution - 1.0), clip.z, clip.w);
					vColor = mix(inColorA, inColorB, inPos.z);
				}
		)");

		mPipelineBuilder->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
			layout(location = 0) in vec4 vColor;
			void main(){
				BaseColor = vColor;
				%1
			})")
#ifdef QENGINE_WITH_EDITOR	
			.arg("DebugId = " + DebugUtils::convertIdToVec4Code(getID()) + ";")
#else
			.arg("")
#endif
			.toLocal8Bit()
		);
	}
	
}

void QSplineRenderComponent::onRebuildPipeline() {
	mPipelineBuilder->create(this);
}

void QSplineRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->updateDynamicBuffer(mInstanceBuffer.get(), 0, mPoints.size() * sizeof(QSplinePoint), mPoints.data());
	batch->uploadStaticBuffer(mVertexBuffer.get(), mSegmentData.data());
}

void QSplineRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	mPipelineBuilder->getUniformBlock("UBO")->setParamValue("MVP", QVariant::fromValue(getMvpMatrix().toGenericMatrix<4, 4>()));
	auto size = getPixelSize();
	mPipelineBuilder->getUniformBlock("UBO")->setParamValue("ScreenResolution", QVector2D(size.width(), size.height()));
	mPipelineBuilder->update(batch);
	if (mPipelineBuilder->sigRebuild.ensure()) {
		mSigRebuildPipeline.request();
	}
}

void QSplineRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	if (mPoints.size() <= 2)
		return;
	cmdBuffer->setGraphicsPipeline(mPipelineBuilder->getGraphicsPipeline());
	cmdBuffer->setViewport(viewport);
	cmdBuffer->setShaderResources();
	const QRhiCommandBuffer::VertexInput vertexBindings[] = {
		{ mInstanceBuffer.get(), 0 },
		{ mVertexBuffer.get(), 0 },
	};
	cmdBuffer->setVertexInput(0, 2, vertexBindings);
	cmdBuffer->draw(mSegmentData.size(), mPoints.size() - 1);
}
