#include "Render/Pass/QBasePassDeferred.h"

QBasePassDeferred::QBasePassDeferred() {

}

QList<QPair<QRhiTexture::Format, QString>> QBasePassDeferred::getRenderTargetSlots() {
	return {
	{QRhiTexture::RGBA32F,"BaseColor"},
	{QRhiTexture::RGBA32F,"Position"},
	{QRhiTexture::RGBA8,"Normal"},
	{QRhiTexture::RGBA8,"Tangent"},
	{QRhiTexture::R8,"Metalness"},
	{QRhiTexture::R8,"Roughness"}
#ifdef QENGINE_WITH_EDITOR
	,{QRhiTexture::RGBA8,"DebugId"}
#endif
	};
}

QRhiRenderPassDescriptor* QBasePassDeferred::getRenderPassDescriptor() {
	return mRT.renderPassDesc.get();
}

QRhiRenderTarget* QBasePassDeferred::getRenderTarget() {
	return mRT.renderTarget.get();
}

void QBasePassDeferred::resizeAndLink(const QSize& size, const TextureLinker& linker) {
	mRT.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atBaseColor->create();
	mRT.atPosition.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atPosition->create();
	mRT.atNormal.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atNormal->create();
	mRT.atTangent.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atTangent->create();
	mRT.atMetalness.reset(mRhi->newTexture(QRhiTexture::R8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atMetalness->create();
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
		QRhiColorAttachment(mRT.atTangent.get()),
		QRhiColorAttachment(mRT.atMetalness.get()),
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
	linker.registerOutputTexture(0, "BaseColor", mRT.atBaseColor.get());
	linker.registerOutputTexture(1, "Position", mRT.atPosition.get());
	linker.registerOutputTexture(2, "Normal", mRT.atNormal.get());
	linker.registerOutputTexture(3, "Tangent", mRT.atTangent.get());
	linker.registerOutputTexture(4, "Metalness", mRT.atMetalness.get());
	linker.registerOutputTexture(5, "Roughness", mRT.atRoughness.get());
#ifdef QENGINE_WITH_EDITOR
	linker.registerOutputTexture(6, "DebugId", mRT.atDebugId.get());
#endif
}