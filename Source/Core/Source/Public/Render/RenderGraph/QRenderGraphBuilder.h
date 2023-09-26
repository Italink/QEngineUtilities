#ifndef QRenderGraphBuilder_h__
#define QRenderGraphBuilder_h__

#include "Render/RHI/QRhiHelper.h"
#include "QRGRhiResourcePool.h"
#include "QEngineCoreAPI.h"

class IRenderPassBuilder;
class IRenderer;

class QENGINECORE_API QRenderGraphBuilder {
public:
	QRenderGraphBuilder(IRenderer* renderer);

	void setupBuffer(QRhiBufferRef& buffer, const QByteArray& name, QRhiBuffer::Type type, QRhiBuffer::UsageFlags usages, int size);

	void setupTexture(QRhiTextureRef& texture, const QByteArray& name, QRhiTexture::Format format, const QSize& pixelSize, int sampleCount = 1, QRhiTexture::Flags flags = {});

	void setupSampler(QRhiSamplerRef& sampler,
		const QByteArray& name,
		QRhiSampler::Filter magFilter,
		QRhiSampler::Filter minFilter,
		QRhiSampler::Filter mipmapMode,
		QRhiSampler::AddressMode addressU,
		QRhiSampler::AddressMode addressV,
		QRhiSampler::AddressMode addressW = QRhiSampler::Repeat);

	void setupShaderResourceBindings(QRhiShaderResourceBindingsRef& bindings, const QByteArray& name, QVector<QRhiShaderResourceBinding> binds);

	void setupRenderBuffer(QRhiRenderBufferRef& renderBuffer,
		const QByteArray& name,
		QRhiRenderBuffer::Type type,
		const QSize& pixelSize,
		int sampleCount = 1,
		QRhiRenderBuffer::Flags flags = {},
		QRhiTexture::Format backingFormatHint = QRhiTexture::UnknownFormat);

	void setupRenderTarget(QRhiTextureRenderTargetRef& renderTarget,
		const QByteArray& name,
		const QRhiTextureRenderTargetDescription& desc,
		QRhiTextureRenderTarget::Flags flags = {});

	void setupGraphicsPipeline(QRhiGraphicsPipelineRef& pipeline,
		const QByteArray& name,
		const QRhiGraphicsPipelineState& state);

	void setupComputePipeline(QRhiComputePipelineRef& pipeline,
		const QByteArray& name,
		const QRhiComputePipelineState& state);


	void addPass(std::function<void(QRhiCommandBuffer*)> executor);

	//template<typename RGPassBuilder>
	//typename RGPassBuilder::OutputParams addPassBuilder(RGPassBuilder* passBuilder, typename RGPassBuilder::InputParams input) {input
	//	passBuilder->Input = input;
	//	passBuilder->setup(*this);
	//	addPass(std::bind(&RGPassBuilder::execute, passBuilder, std::placeholders::_1));
	//	return passBuilder->Output;
	//}

	template<typename RGPassBuilder>
	typename RGPassBuilder::Input& addPassBuilder(RGPassBuilder* passBuilder) {
		passBuilder->mInput.mPassBuilder = passBuilder;
		passBuilder->mInput.mRGBuilder = this;
		return passBuilder->mInput;
	}


	QRhi* rhi() const;
	const QShader& fullScreenVS();
	IRenderer* renderer() const;
	QRhiRenderTarget* mainRenderTarget() const;

	void setMainRenderTarget(QRhiRenderTarget* renderTarget);
public:
	void compile();
	void execute(QRhiCommandBuffer* cmdBuffer);
private:
	QRhi* mRhi;
	IRenderer* mRenderer = nullptr;
	QRhiRenderTarget* mMainRenderTarget = nullptr;
	QShader mFullScreenVertexShader;
	QScopedPointer<QRGRhiResourcePool> mResourcePool;
	QVector<std::function<void(QRhiCommandBuffer*)>> mExecutors;
};

#endif // QRenderGraphBuilder_h__
