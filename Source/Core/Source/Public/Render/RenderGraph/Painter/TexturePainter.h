#ifndef TexturePainter_h__
#define TexturePainter_h__

#include "Render/RenderGraph/Painter/IPainter.h"
#include "QEngineCoreAPI.h"

class QENGINECORE_API TexturePainter :public IPainter {
public:
	TexturePainter();

	void setupTexture(QRhiTexture* texture);
	QRhiTexture* getTexture() { return mTexture; }

	void setupSampler(QRhiSampler::Filter magFilter = QRhiSampler::Linear,
		QRhiSampler::Filter minFilter = QRhiSampler::Linear,
		QRhiSampler::Filter mipmapMode = QRhiSampler::None,
		QRhiSampler::AddressMode addressU = QRhiSampler::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::Repeat);

	void setup(QRenderGraphBuilder& builder, QRhiRenderTarget* rt) override;
	void resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi) override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
protected:
	QRhiTexture* mTexture = nullptr;

	QShader mTextureFS;
	QRhiSamplerRef mSampler;
	QRhiSampler::Filter mMagFilter = QRhiSampler::Linear;
	QRhiSampler::Filter mMinFilter = QRhiSampler::Linear;
	QRhiSampler::Filter mMipmapMode = QRhiSampler::None;
	QRhiSampler::AddressMode mAddressU = QRhiSampler::Repeat;
	QRhiSampler::AddressMode mAddressV = QRhiSampler::Repeat;
	QRhiSampler::AddressMode mAddressW = QRhiSampler::Repeat;

	QRhiGraphicsPipelineRef mPipeline;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // TexturePainter_h__
