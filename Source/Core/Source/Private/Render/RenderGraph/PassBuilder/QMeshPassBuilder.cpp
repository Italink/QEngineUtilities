#include "QMeshPassBuilder.h"
#include "Render/RenderGraph/IRenderPassBuilder.h"
#include "Render/IRenderComponent.h"

void QMeshPassBuilder::setup(QRenderGraphBuilder& builder){
	builder.setupTexture(mOutput.BaseColor, "BaseColor", QRhiTexture::Format::RGBA8, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderBuffer(mDepthStencilBuffer, "DepthStencil", QRhiRenderBuffer::DepthStencil, builder.getMainRenderTarget()->pixelSize(), 1);
	builder.setupRenderTarget(mRenderTarget, "MeshPassRT", QRhiTextureRenderTargetDescription(mOutput.BaseColor.get(), mDepthStencilBuffer.get()));

	IMeshPassBuilder::setup(builder);
}

QRhiTextureRenderTarget* QMeshPassBuilder::renderTarget()
{
	return mRenderTarget.get();
}
