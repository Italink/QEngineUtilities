#include "IRenderComponent.h"

void IRenderComponent::initialize(IRenderer* renderer, QRhiTextureRenderTarget* renderTarget)
{
	mRhi = renderer->rhi();
	mRenderer = renderer;
	mRenderTarget = renderTarget;
	mSigRebuildResource.request();
}

QSharedPointer<QPrimitiveRenderProxy> IRenderComponent::newPrimitiveRenderProxy()
{
	return QSharedPointer<QPrimitiveRenderProxy>::create(this);
}

bool IRenderComponent::hasColorAttachment(const QString& name)
{
	for (int i = 0; i < mRenderTarget->description().colorAttachmentCount(); i++) {
		auto colorAttach = mRenderTarget->description().colorAttachmentAt(i);
		if (colorAttach->texture()->name() == name)
			return true;
	}
	return false;
}

int IRenderComponent::getColorAttachmentCount()
{
	return mRenderTarget->description().colorAttachmentCount();
}

int IRenderComponent::getSampleCount() const
{
	return mRenderTarget->sampleCount();
}

QSize IRenderComponent::getPixelSize() const
{
	return mRenderTarget->pixelSize();
}

QRhiRenderPassDescriptor* IRenderComponent::getRenderPassDesc() const
{
	return mRenderTarget->renderPassDescriptor();
}

QRhi* IRenderComponent::getRhi() const
{
	return mRhi;
}

IRenderer* IRenderComponent::getRenderer() const
{
	return mRenderer;
}

QRhiTextureRenderTarget* IRenderComponent::getRenderTarget() const
{
	return mRenderTarget;
}
