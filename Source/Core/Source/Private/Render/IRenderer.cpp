#include "Render/IRenderer.h"
#include "Render/IRenderPass.h"
#include "Render/Painter/TexturePainter.h"

IRenderer::IRenderer(QRhiEx* inRhi, const QSize& inFrameSize)
	: mRhi(inRhi)
	, mFrameSize(inFrameSize)
	, mOutputPainter(new TexturePainter)
{
	mOutputPainter->setupRhi(mRhi);
}

IRenderer::~IRenderer() {

}

void IRenderer::requestComplie() {
	bRequestCompile = true;
}

void IRenderer::compile() {
	bRequestCompile = false;
	if (!mFrameGraph)
		return;
	mFrameGraph->compile(this);
	resize(mFrameSize);
}

void IRenderer::render() {
	if (!mFrameGraph)
		return;
	if (bRequestCompile) {
		compile();
		bRequestCompile = false;
	}
	mFrameGraph->render(commandBuffer());
}

void IRenderer::setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph) {
	mFrameGraph = inFrameGraph;
}

void IRenderer::setCurrentObject(QObject* val) {
	if (mCurrentObject != val) {
		mCurrentObject = val;
		Q_EMIT asCurrentObjectChanged(mCurrentObject);
	}
}

void IRenderer::resize(const QSize& size) {
	mFrameSize = size;
	if (!mFrameGraph)
		return;
	mOutputTexture = nullptr;
	mFrameGraph->resize(size);
	mOutputTexture = mFrameGraph->getOutputTexture();
	mOutputPainter->setupSampleCount(sampleCount());
	mOutputPainter->setupRenderPassDesc(renderTaget()->renderPassDescriptor());
	mOutputPainter->setupTexture(mOutputTexture);
	mOutputPainter->compile();
}

void IRenderer::setCamera(QCamera* inCamera) {
	mCamera = inCamera;
}

IRenderComponent* IRenderer::getComponentById(uint32_t inId) {
	for (const auto& component : this->findChildren<IRenderComponent*>()) {
		if (component->getID() == inId) {
			return component;
		}
	}
	return nullptr;
}

QRhiTexture* IRenderer::getTexture(const QString& inPassName, int inSlot) {
	if (mFrameGraph) {
		return mFrameGraph->getOutputTexture(inPassName, inSlot);
	}
	return nullptr;
}

#include <moc_IRenderer.cpp>