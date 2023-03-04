﻿#ifndef IRenderPassBase_h__
#define IRenderPassBase_h__

#include "Render/RHI/QRhiEx.h"
#include "IRenderer.h"
#include "IRenderComponent.h"
#include "Utils/QMetaData.h"

struct TextureLinker {
	TextureLinker(IRenderPassBase* pass) :mRenderPass(pass) {};
	QRhiTexture* getInputTexture(int slot) const;
	void registerOutputTexture(int slot, const QByteArray& name, QRhiTexture* texture) const;
private:
	IRenderPassBase* mRenderPass = nullptr;
};

struct InputTextureLinkInfo {
	QString passName;
	int passSlot;
	QRhiTexture* cache = nullptr;
};

class IRenderPassBase: public QObject{
	friend class QFrameGraph;
	friend struct TextureLinker;
public:
	virtual void setRenderer(IRenderer* inRenderer);

	IRenderer* getRenderer() const { return mRenderer; }

	virtual void compile() {};

	virtual void resizeAndLink(const QSize& size, const TextureLinker& linker) {}

	virtual void render(QRhiCommandBuffer* cmdBuffer) = 0;

	virtual QRhiTexture* getOutputTexture(int slot = 0);

	const QMap<int, QRhiTexture*>& getOutputTextures();

	IRenderPassBase* setupInputTexture(int inInputSlot, const QString& inPassName, int inPassSlot);

	QStringList getDependentRenderPassNames();

	const QMap<int, InputTextureLinkInfo>& getInputTextureLinks() { return mInputTextureLinks; }

	void cleanupInputLinkerCache();
protected:
	IRenderer* mRenderer = nullptr;
	QRhiEx* mRhi;
	QMap<int, InputTextureLinkInfo> mInputTextureLinks;
	QMap<int, QRhiTexture*> mOutputTextures;
};

class ISceneRenderPass :public IRenderPassBase {
	Q_OBJECT
public:
	void setSampleCount(int inSampleCount) { mSampleCount = inSampleCount;}
	int getSampleCount() { return mSampleCount; }

	virtual QList<QPair<QRhiTexture::Format, QString>> getRenderTargetSlots() {
		return { {QRhiTexture::RGBA8,"BaseColor"} };
	};
	virtual QRhiRenderPassDescriptor* getRenderPassDescriptor() = 0;
	virtual QRhiRenderTarget* getRenderTarget() = 0;
	void setRenderer(IRenderer* inRenderer) override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
	ISceneRenderPass* addRenderComponent(IRenderComponent* inRenderComponent);
protected:
	QVector<IRenderComponent*> mRenderComponents;
	int mSampleCount = 1;
};

#endif // IRenderPassBase_h__