#ifndef TexturePainter_h__
#define TexturePainter_h__

#include "Render/IPainter.h"

class TexturePainter :public IPainter {
public:
	TexturePainter();

	void setupTexture(QRhiTexture* texture);
	QRhiTexture* getTexture() { return mTexture; }

	void setupSampler(QRhiSampler::Filter magFilter = QRhiSampler::Linear,
		QRhiSampler::Filter minFilter = QRhiSampler::Linear,
		QRhiSampler::Filter mipmapMode = QRhiSampler::Linear,
		QRhiSampler::AddressMode addressU = QRhiSampler::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::Repeat);

	virtual void compile() override;
	virtual void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
protected:
	QRhiTexture* mTexture = nullptr;

	QScopedPointer<QRhiSampler> mSampler;
	QRhiSampler::Filter mMagFilter = QRhiSampler::Linear;
	QRhiSampler::Filter mMinFilter = QRhiSampler::Linear;
	QRhiSampler::Filter mMipmapMode = QRhiSampler::Linear;
	QRhiSampler::AddressMode mAddressU = QRhiSampler::Repeat;
	QRhiSampler::AddressMode mAddressV = QRhiSampler::Repeat;
	QRhiSampler::AddressMode mAddressW = QRhiSampler::Repeat;

	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
};

#endif // TexturePainter_h__
