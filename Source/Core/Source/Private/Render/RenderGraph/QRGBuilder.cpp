#include "QRGBuilder.h"
#include "IRGPassBuilder.h"
#include "IRenderer.h"

QRGBuilder::QRGBuilder(IRenderer* renderer)
{
	mRhi = renderer->rhi();
	mRenderer = renderer;
	mResourcePool.reset(new QRGRhiResourcePool(mRhi));
}

void QRGBuilder::setupBuffer(QRhiBufferRef& buffer, const QByteArray& name, QRhiBuffer::Type type, QRhiBuffer::UsageFlags usages, int size)
{
	if (buffer) {
		buffer->setType(type);
		buffer->setUsage(usages);
		buffer->setSize(size);
		mResourcePool->checkValidity(buffer);
	}
	else {
		buffer = mResourcePool->findOrNew(type, usages, size);
	}
	buffer->setName(name);
}

void QRGBuilder::setupTexture(QRhiTextureRef& texture, const QByteArray& name, QRhiTexture::Format format, const QSize& pixelSize, int sampleCount, QRhiTexture::Flags flags)
{
	if (texture) {
		texture->setFormat(format);
		texture->setPixelSize(pixelSize);
		texture->setSampleCount(sampleCount);
		texture->setFlags(flags);
		mResourcePool->checkValidity(texture);
	}
	else {
		texture = mResourcePool->findOrNew(format, pixelSize, sampleCount, flags);
	}
	texture->setName(name);
}

void QRGBuilder::setupSampler(QRhiSamplerRef& sampler, const QByteArray& name, QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW)
{
	if (sampler) {
		sampler->setMagFilter(magFilter);
		sampler->setMinFilter(minFilter);
		sampler->setMipmapMode(mipmapMode);
		sampler->setAddressU(addressU);
		sampler->setAddressV(addressV);
		sampler->setAddressW(addressW);
		mResourcePool->checkValidity(sampler);
	}
	else {
		sampler = mResourcePool->findOrNew(magFilter, minFilter, mipmapMode, addressU, addressV, addressW);
	}
	sampler->setName(name);
}

void QRGBuilder::setupShaderResourceBindings(QRhiShaderResourceBindingsRef& bindings, const QByteArray& name, QVector<QRhiShaderResourceBinding> binds)
{
	if (bindings) {
		bindings->setBindings(binds.begin(),binds.end());
		mResourcePool->checkValidity(bindings);
	}
	else {
		bindings = mResourcePool->findOrNew(binds);
	}
	bindings->setName(name);
}

void QRGBuilder::setupRenderBuffer(QRhiRenderBufferRef& renderBuffer, const QByteArray& name, QRhiRenderBuffer::Type type, const QSize& pixelSize, int sampleCount, QRhiRenderBuffer::Flags flags, QRhiTexture::Format backingFormatHint)
{
	if (renderBuffer && renderBuffer->backingFormat() == backingFormatHint) {
		renderBuffer->setType(type);
		renderBuffer->setPixelSize(pixelSize);
		renderBuffer->setSampleCount(sampleCount);
		renderBuffer->setFlags(flags);
		mResourcePool->checkValidity(renderBuffer);
	}
	else {
		renderBuffer = mResourcePool->findOrNew(type, pixelSize, sampleCount, flags, backingFormatHint);
	}
	renderBuffer->setName(name);
}

void QRGBuilder::setupRenderTarget(QRhiTextureRenderTargetRef& renderTarget, const QByteArray& name, const QRhiTextureRenderTargetDescription& desc, QRhiTextureRenderTarget::Flags flags)
{
	if (renderTarget) {
		renderTarget->setDescription(desc);
		renderTarget->setFlags(flags);
		mResourcePool->checkValidity(renderTarget);
	}
	else {
		renderTarget = mResourcePool->findOrNew(desc, flags);
	}
	renderTarget->setName(name);
}

void QRGBuilder::setupGraphicsPipeline(QRhiGraphicsPipelineRef& pipeline, const QByteArray& name, const QRhiGraphicsPipelineState& state)
{
	if (pipeline) {
		state.assignTo(pipeline.get());
		mResourcePool->checkValidity(pipeline);
	}
	else {
		pipeline = mResourcePool->findOrNew(state);
	}
	pipeline->setName(name);
}

void QRGBuilder::setupComputePipeline(QRhiComputePipelineRef& pipeline, const QByteArray& name, const QRhiComputePipelineState& state)
{
	if (pipeline) {
		state.assignTo(pipeline.get());
		mResourcePool->checkValidity(pipeline);
	}
	else {
		pipeline = mResourcePool->findOrNew(state);
	}
	pipeline->setName(name);
}

void QRGBuilder::addPass(std::function<void(QRhiCommandBuffer*)> executor)
{
	mExecutors << executor;
}

QRhi* QRGBuilder::rhi() const
{
	return mRhi;
}

IRenderer* QRGBuilder::renderer() const
{
	return mRenderer;
}

void QRGBuilder::setMainRenderTarget(QRhiRenderTarget* renderTarget)
{
	mMainRenderTarget = renderTarget;
}

QRhiRenderTarget* QRGBuilder::mainRenderTarget() const
{
	return mMainRenderTarget;
}

void QRGBuilder::compile()
{
	mResourcePool->recreateBuffers();
	mResourcePool->recreateTextures();
	mResourcePool->recreateRenderBuffers();
	mResourcePool->recreateSamplers();
	mResourcePool->recreateBindings();
	mResourcePool->recreateRenderTargets();
	mResourcePool->recreateGraphicsPipelines();
	mResourcePool->recreateComputePipelines();
}

void QRGBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	for (auto& executor : mExecutors) {
		executor(cmdBuffer);
	}
	mExecutors.clear();
}

