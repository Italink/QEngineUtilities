#include "IRenderer.h"
#include "IRenderPass.h"
#include "Render/Painter/TexturePainter.h"

IRenderer::IRenderer(QRhiEx* inRhi, const QSize& inFrameSize)
	: mRhi(inRhi)
	, mFrameSize(inFrameSize)
	, mOutputPainter(new TexturePainter)
{
	mOutputPainter->setupRhi(mRhi);
}

void IRenderer::requestComplie() {
	bRequestCompile = true;
}

void IRenderer::compile() {
	if (!mFrameGraph)
		return;
	mFrameGraph->rebuildTopology();
	for (auto& renderPass : mFrameGraph->getRenderPassTopology()) {
		renderPass->setRenderer(this);
		TextureLinker linker(renderPass);
		renderPass->resizeAndLink(renderTaget()->pixelSize(), linker);
		renderPass->compile();
	}
	refreshOutputTexture();
}

void IRenderer::render() {
	if (!mFrameGraph)
		return;
	if (bRequestCompile) {
		compile();
		bRequestCompile = false;
	}

	QRhiCommandBuffer* cmdBuffer = commandBuffer();
	for (auto& renderPass : mFrameGraph->getRenderPassTopology()) {
		renderPass->render(cmdBuffer);
	}

	cmdBuffer->beginPass(renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	mOutputPainter->paint(cmdBuffer, renderTaget());
	cmdBuffer->endPass();
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
	for (auto& renderPass : mFrameGraph->getRenderPassTopology()) {
		renderPass->cleanupInputLinkerCache();
	}
	for (auto& renderPass : mFrameGraph->getRenderPassTopology()) {
		TextureLinker linker(renderPass);
		renderPass->resizeAndLink(size, linker);
	}
	refreshOutputTexture();
}

void IRenderer::refreshOutputTexture() {
	QRhiTexture* texture = mFrameGraph->getOutputTexture();
	mOutputPainter->setupSampleCount(sampleCount());
	mOutputPainter->setupRenderPassDesc(renderTaget()->renderPassDescriptor());
	mOutputPainter->setupTexture(texture);
	mOutputPainter->compile();
}

void IRenderer::setCamera(QCamera* inCamera) {
	mCamera = inCamera;
}

IRenderPassBase* IRenderer::getRenderPassByName(const QString& inName) {
	return mFrameGraph ? mFrameGraph->getRenderPassMap().value(inName):nullptr;
}

IRenderComponent* IRenderer::getComponentById(uint32_t inId) {
	for (const auto& component : this->findChildren<IRenderComponent*>()) {
		if (component->getID() == inId) {
			return component;
		}
	}
	return nullptr;
}
