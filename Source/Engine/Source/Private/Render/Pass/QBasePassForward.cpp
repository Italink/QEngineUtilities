//#include "Render/Pass/QBasePassForward.h"
//
//QBasePassForward::QBasePassForward() {
//
//}
//
//QRhiRenderPassDescriptor* QBasePassForward::getRenderPassDescriptor() {
//	return mRT.renderPassDesc.get();
//}
//
//QRhiRenderTarget* QBasePassForward::getRenderTarget() {
//	return mRT.renderTarget.get();
//}
//
//void QBasePassForward::resizeAndLinkNode(const QSize& size) {
//	mRT.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size,getSampleCount() , QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
//	mRT.atBaseColor->create();
//	registerTextureOut_BaseColor(mRT.atBaseColor.get());
//
//#ifdef QENGINE_WITH_EDITOR
//	mRT.atDebugId.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, getSampleCount(), QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
//	mRT.atDebugId->create();
//	registerTextureOut_DebugId(mRT.atDebugId.get());
//#endif
//
//	mRT.atDepthStencil.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::Type::DepthStencil, size, getSampleCount()));
//	mRT.atDepthStencil->create();
//
//	QRhiTextureRenderTargetDescription RTDesc;
//	RTDesc.setColorAttachments({
//		QRhiColorAttachment(mRT.atBaseColor.get()),
//#ifdef QENGINE_WITH_EDITOR
//		QRhiColorAttachment(mRT.atDebugId.get()),
//#endif
//	});
//
//	RTDesc.setDepthStencilBuffer(mRT.atDepthStencil.get());
//	mRT.renderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
//	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
//	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
//	mRT.renderTarget->create();
//}