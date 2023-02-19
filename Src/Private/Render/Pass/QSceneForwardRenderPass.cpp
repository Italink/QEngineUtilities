#include "Render/Pass/QSceneForwardRenderPass.h"

QSceneForwardRenderPass::QSceneForwardRenderPass() {

}

QList<QPair<QRhiTexture::Format, QString>> QSceneForwardRenderPass::getRenderTargetSlots() {
	return { 
	{QRhiTexture::RGBA32F,"BaseColor"}
#ifdef QENGINE_WITH_EDITOR
	,{QRhiTexture::RGBA8,"DebugId"}
#endif
	};
}

QRhiRenderPassDescriptor* QSceneForwardRenderPass::getRenderPassDescriptor() {
	return mRT.renderPassDesc.get();
}

QRhiRenderTarget* QSceneForwardRenderPass::getRenderTarget() {
	return mRT.renderTarget.get();
}

void QSceneForwardRenderPass::resizeAndLink(const QSize& size, const TextureLinker& linker) {
	mRT.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size,getSampleCount() , QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atBaseColor->create();
#ifdef QENGINE_WITH_EDITOR
	mRT.atDebugId.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atDebugId->create();
#endif
	mRT.atDepthStencil.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::Type::DepthStencil, size, getSampleCount()));
	mRT.atDepthStencil->create();
	QRhiTextureRenderTargetDescription RTDesc;
	RTDesc.setColorAttachments({
		QRhiColorAttachment(mRT.atBaseColor.get()),
#ifdef QENGINE_WITH_EDITOR
		QRhiColorAttachment(mRT.atDebugId.get()),
#endif
		});
	RTDesc.setDepthStencilBuffer(mRT.atDepthStencil.get());
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	linker.registerOutputTexture(0, "BaseColor", mRT.atBaseColor.get());
#ifdef QENGINE_WITH_EDITOR
	linker.registerOutputTexture(1, "DebugId", mRT.atDebugId.get());
#endif
}