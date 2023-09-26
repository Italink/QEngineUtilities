#include "QMeshPassBuilder.h"

void QMeshPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mOutput.BaseColor, "BaseColor", QRhiTexture::Format::RGBA8, builder.mainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderBuffer(mDepthStencilBuffer, "DepthStencil", QRhiRenderBuffer::DepthStencil, builder.mainRenderTarget()->pixelSize(), 1);
	builder.setupRenderTarget(mRenderTarget, "MeshPassRT", QRhiTextureRenderTargetDescription(mOutput.BaseColor.get(), mDepthStencilBuffer.get()));
	for (auto& component : mInput._Components) {
		if (component->getRhi() != builder.rhi()) {
			component->initialize(builder.renderer(), mRenderTarget.get());
		}
	}
}

void QMeshPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	QVector<IRenderComponent*> uploadItems;
	for (auto& item : mInput._Components) {
		if (item->mSigRebuildResource.ensure()) {
			item->onRebuildResource();
			uploadItems << item;
		}
		if (item->mSigRebuildPipeline.ensure()) {
			item->onRebuildPipeline();
		}
		item->onPreUpdate(cmdBuffer);
	}
	QRhiResourceUpdateBatch* resUpdateBatch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	for (auto& item : uploadItems) {
		item->onUpload(resUpdateBatch);
	}
	for (auto& item : mInput._Components) {
		item->onUpdate(resUpdateBatch);
	}
	cmdBuffer->beginPass(mRenderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), {1.0f, 0}, resUpdateBatch, QRhiCommandBuffer::ExternalContent);
	QRhiViewport viewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height());
	for (auto& item : mInput._Components) {
		item->onRender(cmdBuffer, viewport);
	}
	cmdBuffer->endPass();
}
