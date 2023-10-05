#include "QPbrMeshPassBuilder.h"

void QPbrMeshPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mOutput.BaseColor, "BaseColor", QRhiTexture::Format::RGBA32F, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupTexture(mOutput.Position, "Position", QRhiTexture::Format::RGBA32F, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupTexture(mOutput.Normal, "Normal", QRhiTexture::Format::RGBA16F, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupTexture(mOutput.Metallic, "Metallic", QRhiTexture::Format::R8, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupTexture(mOutput.Roughness, "Roughness", QRhiTexture::Format::R8, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupTexture(mOutput.Depth, "Depth", QRhiTexture::Format::D32F, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);

	QRhiTextureRenderTargetDescription rtDesc;
	rtDesc.setColorAttachments({
		QRhiColorAttachment(mOutput.BaseColor.get()),
		QRhiColorAttachment(mOutput.Position.get()),
		QRhiColorAttachment(mOutput.Normal.get()),
		QRhiColorAttachment(mOutput.Metallic.get()),
		QRhiColorAttachment(mOutput.Roughness.get()),
	});
	rtDesc.setDepthTexture(mOutput.Depth.get());

	builder.setupRenderTarget(mRenderTarget, "PbrMeshPassRT", rtDesc);

	IMeshPassBuilder::setup(builder);
}

QRhiTextureRenderTarget* QPbrMeshPassBuilder::renderTarget()
{
	return mRenderTarget.get();
}