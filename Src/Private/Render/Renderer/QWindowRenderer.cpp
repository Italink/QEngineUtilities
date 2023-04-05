#include "Render/Renderer/QWindowRenderer.h"
#include "Render/RHI/QRhiWindow.h"
#include "Render/IRenderPass.h"

#ifdef QENGINE_WITH_EDITOR
#include "Render/Painter/DebugUiPainter.h"
#endif // QENGINE_WITH_EDITOR

QWindowRenderer::QWindowRenderer(QRhiWindow* inWindow)
	: IRenderer(inWindow->mRhi.get(), inWindow->mSwapChain->currentPixelSize())
	, mWindow(inWindow)
#ifdef QENGINE_WITH_EDITOR
	, mDebugUiPainter(new QDebugUIPainter(this))
#endif
{
}

QWindow* QWindowRenderer::getWindow() {
	return mWindow;
}

void QWindowRenderer::render() {
	if (!mFrameGraph)
		return;
	if (bRequestCompile) {
		compile();
		bRequestCompile = false;
	}
	QRhiCommandBuffer* cmdBuffer = commandBuffer();
	mFrameGraph->render(cmdBuffer);
#ifdef QENGINE_WITH_EDITOR
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	mDebugUiPainter->resourceUpdate(batch);
	cmdBuffer->beginPass(renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	mOutputPainter->paint(cmdBuffer, renderTaget());
	mDebugUiPainter->paint(cmdBuffer,renderTaget());
	cmdBuffer->endPass();
#else
	cmdBuffer->beginPass(renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	mOutputPainter->paint(cmdBuffer, renderTaget());
	cmdBuffer->endPass();
#endif
}

void QWindowRenderer::resize(const QSize& size) {
	IRenderer::resize(size);
#ifdef QENGINE_WITH_EDITOR
	for (const auto& basePass : findChildren<IBasePass*>()) {
		if (QRhiTexture* debugIdTexture = basePass->getOutputTexture("DebugId")) {
			mDebugUiPainter->setupDebugIdTexture(debugIdTexture);
			break;
		}
	}
	mDebugUiPainter->compile();
#endif // QENGINE_WITH_EDITOR
}

QRhiCommandBuffer* QWindowRenderer::commandBuffer() {
	return mWindow->mSwapChain->currentFrameCommandBuffer();
}

QRhiRenderTarget* QWindowRenderer::renderTaget(){
	return mWindow->mSwapChain->currentFrameRenderTarget();
}

int QWindowRenderer::sampleCount()
{
	return mWindow->mSwapChain->sampleCount();
}

#ifdef QENGINE_WITH_EDITOR
void QWindowRenderer::setOverrideOutput(QRhiTexture* inTexture) {
	if (!inTexture) {
		inTexture = mFrameGraph->getOutputTexture();
	}
	if (inTexture != mOutputPainter->getTexture()) {
		mOutputPainter->setupTexture(inTexture);
		mOutputPainter->compile();
	}
}
#endif

QRhiWindow* QWindowRenderer::getRhiWindow() const {
	return mWindow;
}
