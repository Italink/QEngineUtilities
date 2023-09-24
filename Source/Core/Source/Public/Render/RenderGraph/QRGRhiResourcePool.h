#ifndef QRGRhiResourcePool_h__
#define QRGRhiResourcePool_h__

#include <QMultiHash>
#include <rhi/qrhi.h>

typedef std::shared_ptr<QRhiBuffer> QRhiBufferRef;
typedef std::shared_ptr<QRhiTexture> QRhiTextureRef;
typedef std::shared_ptr<QRhiSampler> QRhiSamplerRef;
typedef std::shared_ptr<QRhiShaderResourceBindings> QRhiShaderResourceBindingsRef;
typedef std::shared_ptr<QRhiRenderBuffer> QRhiRenderBufferRef;
typedef std::shared_ptr<QRhiTextureRenderTarget> QRhiTextureRenderTargetRef;
typedef std::shared_ptr<QRhiRenderTarget> QRhiRenderTargetRef;
typedef std::shared_ptr<QRhiGraphicsPipeline> QRhiGraphicsPipelineRef;
typedef std::shared_ptr<QRhiComputePipeline> QRhiComputePipelineRef;
typedef std::shared_ptr<QRhiRenderPassDescriptor>  QRhiRenderPassDescriptorRef;

struct QRhiGraphicsPipelineState {
	QRhiGraphicsPipeline::Flags flags;
	QRhiGraphicsPipeline::Topology topology = QRhiGraphicsPipeline::Triangles;
	QRhiGraphicsPipeline::CullMode cullMode = QRhiGraphicsPipeline::None;
	QRhiGraphicsPipeline::FrontFace frontFace = QRhiGraphicsPipeline::CCW;
	QVarLengthArray<QRhiGraphicsPipeline::TargetBlend, 8> targetBlends;
	bool depthTest = false;
	bool depthWrite = false;
	QRhiGraphicsPipeline::CompareOp depthOp = QRhiGraphicsPipeline::Less;
	bool stencilTest = false;
	QRhiGraphicsPipeline::StencilOpState stencilFront;
	QRhiGraphicsPipeline::StencilOpState stencilBack;
	quint32 stencilReadMask = 0xFF;
	quint32 stencilWriteMask = 0xFF;
	int sampleCount = 1;
	float lineWidth = 1.0f;
	int depthBias = 0;
	float slopeScaledDepthBias = 0.0f;
	int patchControlPointCount = 3;
	QRhiGraphicsPipeline::PolygonMode polygonMode = QRhiGraphicsPipeline::Fill;
	QVarLengthArray<QRhiShaderStage, 4> shaderStages;
	QRhiVertexInputLayout vertexInputLayout;
	QRhiShaderResourceBindings* shaderResourceBindings = nullptr;
	QRhiRenderPassDescriptor* renderPassDesc = nullptr;
	static QRhiGraphicsPipelineState createFrom(QRhiGraphicsPipeline* pipeline);
	void assignTo(QRhiGraphicsPipeline* pipeline) const;
};

struct QRhiComputePipelineState {
	QRhiComputePipeline::Flags flags;
	QRhiShaderStage shaderStage;
	QRhiShaderResourceBindings* shaderResourceBindings = nullptr;

	static QRhiComputePipelineState createFrom(QRhiComputePipeline* pipeline);
	void assignTo(QRhiComputePipeline* pipeline) const;
};

class QRGRhiResourcePool {
public:
	QRGRhiResourcePool(QRhi* rhi);

	static size_t hash(QRhiBuffer::Type type, QRhiBuffer::UsageFlags usage, quint32 size);
	static size_t hash(QRhiTexture::Format format, const QSize& pixelSize, int sampleCount, QRhiTexture::Flags flags);
	static size_t hash(QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW);
	static size_t hash(QVector<QRhiShaderResourceBinding> bindings);
	static size_t hash(QRhiRenderBuffer::Type type, const QSize& pixelSize, int sampleCount, QRhiRenderBuffer::Flags flags, QRhiTexture::Format backingFormatHint);
	static size_t hash(const QRhiTextureRenderTargetDescription& desc, QRhiTextureRenderTarget::Flags flags);
	static size_t hash(const QRhiGraphicsPipelineState& state);
	static size_t hash(const QRhiComputePipelineState& state);

	QRhiBufferRef findOrNew(QRhiBuffer::Type type, QRhiBuffer::UsageFlags usage, quint32 size);
	QRhiTextureRef findOrNew(QRhiTexture::Format format, const QSize& pixelSize, int sampleCount, QRhiTexture::Flags flags);
	QRhiSamplerRef findOrNew(QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW);
	QRhiShaderResourceBindingsRef findOrNew(QVector<QRhiShaderResourceBinding> bindings);
	QRhiRenderBufferRef findOrNew(QRhiRenderBuffer::Type type, const QSize& pixelSize, int sampleCount, QRhiRenderBuffer::Flags flags, QRhiTexture::Format backingFormatHint);
	QRhiTextureRenderTargetRef findOrNew(const QRhiTextureRenderTargetDescription& desc, QRhiTextureRenderTarget::Flags flags);
	QRhiGraphicsPipelineRef findOrNew(const QRhiGraphicsPipelineState& state);
	QRhiComputePipelineRef findOrNew(const QRhiComputePipelineState& state);

	void checkValidity(QRhiBufferRef res);
	void checkValidity(QRhiTextureRef res);
	void checkValidity(QRhiSamplerRef res);
	void checkValidity(QRhiShaderResourceBindingsRef res);
	void checkValidity(QRhiRenderBufferRef res);
	void checkValidity(QRhiTextureRenderTargetRef res);
	void checkValidity(QRhiGraphicsPipelineRef res);
	void checkValidity(QRhiComputePipelineRef res);

	void recreateBuffers();
	void recreateTextures();
	void recreateSamplers();
	void recreateBindings();
	void recreateRenderBuffers();
	void recreateRenderTargets();
	void recreateGraphicsPipelines();
	void recreateComputePipelines();
private:
	bool fixupResHash(QRhiResource* res, size_t newHashCode);
protected:
	QRhi* mRhi = nullptr;
	QMultiHash<size_t, QRhiBufferRef> mBufferPool;
	QMultiHash<size_t, QRhiTextureRef> mTexturePool;
	QMultiHash<size_t, QRhiSamplerRef> mSamplerPool;
	QMultiHash<size_t, QRhiShaderResourceBindingsRef> mBindingsPool;
	QMultiHash<size_t, QRhiRenderBufferRef> mRenderBufferPool;
	QMultiHash<size_t, QRhiTextureRenderTargetRef> mRenderTargetPool;
	QMultiHash<size_t, QRhiGraphicsPipelineRef> mGraphicsPipelinePool;
	QMultiHash<size_t, QRhiComputePipelineRef> mComputePipelinePool;
	QHash<QRhiRenderTarget*, QRhiRenderPassDescriptorRef> mRenderPassDescPool;

	QHash<QRhiResource*, size_t> mRhiHashMap;
	QList<QRhiBuffer*> mBufferToRecreate;
	QList<QRhiTexture*> mTextureToRecreate;
	QList<QRhiSampler*> mSamplerToRecreate;
	QList<QRhiShaderResourceBindings*> mBindingsToRecreate;
	QList<QRhiRenderBuffer*> mRenderBufferToRecreate;
	QList<QRhiTextureRenderTarget*> mRenderTargetToRecreate;
	QList<QRhiGraphicsPipeline*> mGraphicsPipelineToRecreate;
	QList<QRhiComputePipeline*> mComputePipelineToRecreate;
	QHash<QRhiRenderPassDescriptor*, QRhiRenderPassDescriptor*> mRenderPassDescToRedirect;
};

#endif // QRGRhiResourcePool_h__