#include "QImGUIPassBuilder.h"
#include "IRenderer.h"

QImGUIPassBuilder::QImGUIPassBuilder() 
	: mPainter(new ImGuiPainter)
{

}

void QImGUIPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mRT.colorAttachment, "BlurTextureH", QRhiTexture::RGBA8,builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "BlurRenderTargetH", { mRT.colorAttachment.get() });

	mPainter->setupWindow(builder.getRenderer()->maybeWindow());
	mPainter->setupPaintFunctor(mInput._PaintFunctor);
	mPainter->setup(builder, mRT.renderTarget.get());

	mOutput.ImGuiTexture = mRT.colorAttachment;
}

void QImGUIPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	mPainter->resourceUpdate(batch, cmdBuffer->rhi());
	cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch);
	mPainter->paint(cmdBuffer, mRT.renderTarget.get());
	cmdBuffer->endPass();
}
