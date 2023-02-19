#include "Render/Renderer/QWindowRenderer.h"
#include "Render/RHI/QRhiWindow.h"

#ifdef QENGINE_WITH_EDITOR
#include "DebugDraw/DebugUiPainter.h"
#endif // QENGINE_WITH_EDITOR

QWindowRenderer::QWindowRenderer(QRhiWindow* inWindow)
	: IRenderer(inWindow->mRhi, inWindow->mSwapChain->currentPixelSize())
	, mWindow(inWindow)
#ifdef QENGINE_WITH_EDITOR
	, mDebugUiPainter(new QDebugUIPainter(this))
#endif
{
}

QWindow* QWindowRenderer::getWindow() const {
	return mWindow;
}

void QWindowRenderer::render() {
#ifdef QENGINE_WITH_EDITOR
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

	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	mDebugUiPainter->resourceUpdate(batch);
	cmdBuffer->beginPass(renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	mOutputPainter->paint(cmdBuffer, renderTaget());
	mDebugUiPainter->paint(cmdBuffer,renderTaget());
	cmdBuffer->endPass();
#else
	IRenderer::render();
#endif // QENGINE_WITH_EDITOR
}

QRhiCommandBuffer* QWindowRenderer::commandBuffer() {
	return mWindow->mSwapChain->currentFrameCommandBuffer();
}

QRhiRenderTarget* QWindowRenderer::renderTaget()
{
	return mWindow->mSwapChain->currentFrameRenderTarget();
}

int QWindowRenderer::sampleCount()
{
	return mWindow->mSwapChain->sampleCount();
}

void QWindowRenderer::refreshOutputTexture() 
{
	IRenderer::refreshOutputTexture();
#ifdef QENGINE_WITH_EDITOR
	QRhiTexture* debugIdTexture = nullptr;
	for (const auto& scenePass : findChildren<ISceneRenderPass*>()) {
		auto texSlots = scenePass->getRenderTargetSlots();
		for (int slotIndex = 0; slotIndex < texSlots.size(); ++slotIndex) {
			if (texSlots[slotIndex].second == "DebugId") {
				debugIdTexture = scenePass->getOutputTexture(slotIndex);
				break;
			}
		}
	}
	mDebugUiPainter->setupDebugIdTexture(debugIdTexture);
	mDebugUiPainter->compile();
#endif // QENGINE_WITH_EDITOR
}

