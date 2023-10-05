#include "QRenderGraphBuilder.h"
#include "IRenderPassBuilder.h"
#include "IRenderer.h"

QRenderGraphBuilder::QRenderGraphBuilder(IRenderer* renderer)
{
	mRhi = renderer->rhi();
	mRenderer = renderer;
	mResourcePool.reset(new QRGRhiResourcePool(mRhi));

	mFullScreenVertexShader = QRhiHelper::newShaderFromCode( QShader::VertexStage, R"(#version 450
		layout (location = 0) out vec2 vUV;
		out gl_PerVertex{
			vec4 gl_Position;
		};
		void main() {
			vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
			gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
#if Y_UP_IN_NDC
			gl_Position.y = - gl_Position.y;
#endif 
		})"
		, QShaderDefinitions()
		.addDefinition("Y_UP_IN_NDC", mRhi->isYUpInNDC())
	);
}

void QRenderGraphBuilder::setupBuffer(QRhiBufferRef& buffer, const QByteArray& name, QRhiBuffer::Type type, QRhiBuffer::UsageFlags usages, int size)
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

void QRenderGraphBuilder::setupTexture(QRhiTextureRef& texture, const QByteArray& name, QRhiTexture::Format format, const QSize& pixelSize, int sampleCount, QRhiTexture::Flags flags)
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

void QRenderGraphBuilder::setupSampler(QRhiSamplerRef& sampler, const QByteArray& name, QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW)
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

void QRenderGraphBuilder::setupShaderResourceBindings(QRhiShaderResourceBindingsRef& bindings, const QByteArray& name, QVector<QRhiShaderResourceBinding> binds)
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

void QRenderGraphBuilder::setupRenderBuffer(QRhiRenderBufferRef& renderBuffer, const QByteArray& name, QRhiRenderBuffer::Type type, const QSize& pixelSize, int sampleCount, QRhiRenderBuffer::Flags flags, QRhiTexture::Format backingFormatHint)
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

void QRenderGraphBuilder::setupRenderTarget(QRhiTextureRenderTargetRef& renderTarget, const QByteArray& name, const QRhiTextureRenderTargetDescription& desc, QRhiTextureRenderTarget::Flags flags)
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

	mActivatedRenderTargets << renderTarget.get();
}

void QRenderGraphBuilder::setupGraphicsPipeline(QRhiGraphicsPipelineRef& pipeline, const QByteArray& name, const QRhiGraphicsPipelineState& state)
{
	if (pipeline) {
		state.assignTo(pipeline.get());
		mResourcePool->checkValidity(pipeline);
	}
	else {
		pipeline = mResourcePool->findOrNew(state);
	}
	pipeline->setName(name);

	for (auto& rt : mActivatedRenderTargets) {
		if (rt->renderPassDescriptor() == state.renderPassDesc) {
			mRenderTargetPipelines[rt] << pipeline.get();
			break;
		}
	}
}

void QRenderGraphBuilder::setupComputePipeline(QRhiComputePipelineRef& pipeline, const QByteArray& name, const QRhiComputePipelineState& state)
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

void QRenderGraphBuilder::addPass(std::function<void(QRhiCommandBuffer*)> executor)
{
	mExecutors << executor;
}

QRhi* QRenderGraphBuilder::getRhi() const
{
	return mRhi;
}

const QShader& QRenderGraphBuilder::getFullScreenVS()
{
	return mFullScreenVertexShader;
}

IRenderer* QRenderGraphBuilder::getRenderer() const
{
	return mRenderer;
}

const QList<QRhiTextureRenderTarget*>& QRenderGraphBuilder::getActivatedRenderTargets() const
{
	return mActivatedRenderTargets;
}

const QMap<QRhiTextureRenderTarget*, QList<QRhiGraphicsPipeline*>>& QRenderGraphBuilder::getRenderTargetPipelines() const
{
	return mRenderTargetPipelines;
}

void QRenderGraphBuilder::setMainRenderTarget(QRhiRenderTarget* renderTarget)
{
	mMainRenderTarget = renderTarget;
}

QRhiRenderTarget* QRenderGraphBuilder::getMainRenderTarget() const
{
	return mMainRenderTarget;
}

void QRenderGraphBuilder::compile()
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

void QRenderGraphBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	for (auto& executor : mExecutors) {
		executor(cmdBuffer);
	}
}

void QRenderGraphBuilder::clear()
{
	mExecutors.clear();
	mActivatedRenderTargets.clear();
	mRenderTargetPipelines.clear();
}

