#include "IRenderPass.h"

void IRenderPass::setRenderer(IRenderer* inRenderer) {
	mRenderer = inRenderer;
	mRhi = inRenderer->getRhi();
	setParent(mRenderer);
}

const QSet<QString>& IRenderPass::getDependentPassNodeNames() {
	return mDependentPassNodeNames;
}

int IRenderPass::getOutputTextureSize() {
	return mOutputTexutres.size();
}

QRhiTexture* IRenderPass::getOutputTexture(const QString& inName) {
	for (auto out : mOutputTexutres) {
		if (out->name() == inName) {
			return out;
		}
	}
	return nullptr;
}

QRhiTexture* IRenderPass::getOutputTexture(const int& inSlot) {
	return mOutputTexutres.value(inSlot);
}

QList<QRhiTexture*> IRenderPass::getOutputTextures() {
	return mOutputTexutres.values();
}

QRhiTexture* IRenderPass::getFirstOutputTexture() {
	if (mOutputTexutres.isEmpty())
		return nullptr;
	return mOutputTexutres.first();
}

const QMap<QString, QPair<QString, int>>& IRenderPass::getInputTextureLinks() {
	return mInputTextureLinks;
}

QRhiTexture* IRenderPass::getInputTexture(const QString& inName) {
	if (mInputTextureLinks.contains(inName)) {
		return mRenderer->getTexture(mInputTextureLinks[inName].first, mInputTextureLinks[inName].second);
	}
	return nullptr;
}

void IRenderPass::registerOutputTexture(int slot, const QString& inName, QRhiTexture* inTexture) {
	if (inTexture) {
		inTexture->setName(inName.toLocal8Bit());
		mOutputTexutres[slot] = inTexture;
	}
	else {
		qWarning() << "invalid output texture: " << inName;
	}
}

void IRenderPass::registerInputTextureLink(const QString& inTexName, const QString& inPassName, int inSlot) {
	mInputTextureLinks[inTexName] = { inPassName,inSlot };
	mDependentPassNodeNames.insert(inPassName);
}

void IBasePass::setRenderer(IRenderer* inRenderer) {
	IRenderPass::setRenderer(inRenderer);
	for (auto& comp : mRenderComponents) {
		comp->mRhi = mRhi;
	}
}

void IBasePass::render(QRhiCommandBuffer* cmdBuffer) {
	QVector<IRenderComponent*> uploadItems;
	for (auto& item : mRenderComponents) {
		if (item->sigonRebuildResource.receive()) {
			item->onRebuildResource();
			uploadItems << item;
		}
		if (item->sigonRebuildPipeline.receive()) {
			item->onRebuildPipeline();
		}
		item->onPreUpdate(cmdBuffer);
	}
	QRhiResourceUpdateBatch* resUpdateBatch = mRhi->nextResourceUpdateBatch();
	for (auto& item : uploadItems) {
		item->onUpload(resUpdateBatch);
	}
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		item->onUpdate(resUpdateBatch);
	}
	cmdBuffer->beginPass(getRenderTarget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 0 }, resUpdateBatch, QRhiCommandBuffer::ExternalContent);
	QRhiViewport viewport(0, 0, getRenderTarget()->pixelSize().width(), getRenderTarget()->pixelSize().height());
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		item->onRender(cmdBuffer, viewport);
	}
	cmdBuffer->endPass();
}

QList<QPair<QRhiTexture::Format, QString>> IBasePass::getRenderTargetColorAttachments() {
	QList<QPair<QRhiTexture::Format, QString>> colorAttachments;
	for (auto out : mOutputTexutres) {
		if (out->format() < QRhiTexture::RGB10A2)
			colorAttachments.append({ out->format(),out->name() });
	}
	return colorAttachments;
}

bool IBasePass::hasColorAttachment(const QString& inName) {
	for (auto out : mOutputTexutres) {
		if (out->name() == inName)
			return true;
	}
	return false;
}

IBasePass* IBasePass::addRenderComponent(IRenderComponent* inRenderComponent) {
	inRenderComponent->setParent(this);
	inRenderComponent->mRhi = mRhi;
	inRenderComponent->mBasePass = this;
	inRenderComponent->sigonRebuildResource.request();
	inRenderComponent->sigonRebuildPipeline.request();
	mRenderComponents.push_back(inRenderComponent);
	return this;
}
