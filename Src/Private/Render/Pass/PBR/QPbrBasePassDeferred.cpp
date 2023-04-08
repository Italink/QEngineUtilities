#include "Render/Pass/PBR/QPbrBasePassDeferred.h"

QPbrBasePassDeferred::QPbrBasePassDeferred() {

}

QRhiRenderPassDescriptor* QPbrBasePassDeferred::getRenderPassDescriptor() {
	return mRT.renderPassDesc.get();
}

QRhiRenderTarget* QPbrBasePassDeferred::getRenderTarget() {
	return mRT.renderTarget.get();
}

void QPbrBasePassDeferred::resizeAndLinkNode(const QSize& size) {
	mRT.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atBaseColor->create();
	mRT.atPosition.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atPosition->create();
	mRT.atNormal.reset(mRhi->newTexture(QRhiTexture::RGBA16F, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atNormal->create();
	mRT.atMetallic.reset(mRhi->newTexture(QRhiTexture::R8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atMetallic->create();
	mRT.atRoughness.reset(mRhi->newTexture(QRhiTexture::R8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atRoughness->create();
#ifdef QENGINE_WITH_EDITOR
	mRT.atDebugId.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atDebugId->create();
#endif
	mRT.atDepthStencil.reset(mRhi->newTexture(QRhiTexture::D32F, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atDepthStencil->create();
	QRhiTextureRenderTargetDescription RTDesc;
	RTDesc.setColorAttachments({
		QRhiColorAttachment(mRT.atBaseColor.get()),
		QRhiColorAttachment(mRT.atPosition.get()),
		QRhiColorAttachment(mRT.atNormal.get()),
		QRhiColorAttachment(mRT.atMetallic.get()),
		QRhiColorAttachment(mRT.atRoughness.get()),
#ifdef QENGINE_WITH_EDITOR
		QRhiColorAttachment(mRT.atDebugId.get()),
#endif
		});
	RTDesc.setDepthTexture(mRT.atDepthStencil.get());
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	registerTextureOut_BaseColor(mRT.atBaseColor.get());
	registerTextureOut_Position(mRT.atPosition.get());
	registerTextureOut_Normal(mRT.atNormal.get());
	registerTextureOut_Metallic(mRT.atMetallic.get());
	registerTextureOut_Roughness(mRT.atRoughness.get());
#ifdef QENGINE_WITH_EDITOR
	registerTextureOut_DebugId(mRT.atDebugId.get());
#endif
}
