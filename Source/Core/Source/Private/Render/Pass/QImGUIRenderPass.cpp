#include "Render/Pass/QImGUIRenderPass.h"
#include "Render/RHI/QRhiWindow.h"

QImGUIRenderPass::QImGUIRenderPass() 
	: mPainter(new ImGuiPainter)
{
}

void QImGUIRenderPass::setPaintFunctor(std::function<void(ImGuiContext*)> val) {
	mPainter->setupPaintFunctor(val);
}

void QImGUIRenderPass::resizeAndLinkNode(const QSize& size) {
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, mRenderer->getFrameSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_UiLayer(mRT.colorAttachment.get());
}

void QImGUIRenderPass::compile() {
	mPainter->setupWindow(mRenderer->getWindow());
	mPainter->setupRhi(mRhi);
	mPainter->setupRenderPassDesc(mRT.renderPassDesc.get());
	mPainter->setupSampleCount(1);
	mPainter->compile();
}

void QImGUIRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	mPainter->resourceUpdate(batch);
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	mPainter->paint(cmdBuffer, mRT.renderTarget.get());
	cmdBuffer->endPass();
}

