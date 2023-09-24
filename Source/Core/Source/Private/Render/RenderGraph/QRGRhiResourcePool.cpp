#include "QRGRhiResourcePool.h"

QRhiGraphicsPipelineState QRhiGraphicsPipelineState::createFrom(QRhiGraphicsPipeline* pipeline)
{
	QRhiGraphicsPipelineState state;
	state.flags = pipeline->flags();
	state.topology = pipeline->topology();
	state.cullMode = pipeline->cullMode();
	state.frontFace = pipeline->frontFace();
	state.targetBlends = QVarLengthArray<QRhiGraphicsPipeline::TargetBlend, 8>(pipeline->cbeginTargetBlends(), pipeline->cendTargetBlends());
	state.depthTest = pipeline->hasDepthTest();
	state.depthWrite = pipeline->hasDepthWrite();
	state.depthOp = pipeline->depthOp();
	state.stencilTest = pipeline->hasStencilTest();
	state.stencilFront = pipeline->stencilFront();
	state.stencilBack = pipeline->stencilBack();
	state.stencilReadMask = pipeline->stencilReadMask();
	state.stencilWriteMask = pipeline->stencilWriteMask();
	state.sampleCount = pipeline->sampleCount();
	state.lineWidth = pipeline->lineWidth();
	state.depthBias = pipeline->depthBias();
	state.slopeScaledDepthBias = pipeline->slopeScaledDepthBias();
	state.patchControlPointCount = pipeline->patchControlPointCount();
	state.polygonMode = pipeline->polygonMode();
	state.shaderStages = QVarLengthArray<QRhiShaderStage, 4>(pipeline->cbeginShaderStages(), pipeline->cendShaderStages());
	state.vertexInputLayout = pipeline->vertexInputLayout();
	state.shaderResourceBindings = pipeline->shaderResourceBindings();
	state.renderPassDesc = pipeline->renderPassDescriptor();
	return state;
}

void QRhiGraphicsPipelineState::assignTo(QRhiGraphicsPipeline* pipeline) const
{
	pipeline->setFlags(flags);
	pipeline->setTopology(topology);
	pipeline->setCullMode(cullMode);
	pipeline->setFrontFace(frontFace);
	pipeline->setTargetBlends(targetBlends.begin(), targetBlends.end());
	pipeline->setDepthTest(depthTest);
	pipeline->setDepthWrite(depthWrite);
	pipeline->setDepthOp(depthOp);
	pipeline->setStencilTest(stencilTest);
	pipeline->setStencilFront(stencilFront);
	pipeline->setStencilBack(stencilBack);
	pipeline->setStencilReadMask(stencilReadMask);
	pipeline->setStencilWriteMask(stencilWriteMask);
	pipeline->setSampleCount(sampleCount);
	pipeline->setLineWidth(lineWidth);
	pipeline->setDepthBias(depthBias);
	pipeline->setSlopeScaledDepthBias(slopeScaledDepthBias);
	pipeline->setPatchControlPointCount(patchControlPointCount);
	pipeline->setPolygonMode(polygonMode);
	pipeline->setShaderStages(shaderStages.begin(),shaderStages.end());
	pipeline->setVertexInputLayout(vertexInputLayout);
	pipeline->setShaderResourceBindings(shaderResourceBindings);
	pipeline->setRenderPassDescriptor(renderPassDesc);
}

QRhiComputePipelineState QRhiComputePipelineState::createFrom(QRhiComputePipeline* pipeline)
{
	QRhiComputePipelineState state;
	state.flags = pipeline->flags();
	state.shaderResourceBindings = pipeline->shaderResourceBindings();
	state.shaderStage = pipeline->shaderStage();
	return state;
}

void QRhiComputePipelineState::assignTo(QRhiComputePipeline* pipeline) const
{
	pipeline->setFlags(flags);
	pipeline->setShaderResourceBindings(shaderResourceBindings);
	pipeline->setShaderStage(shaderStage);
}

QRGRhiResourcePool::QRGRhiResourcePool(QRhi* rhi)
{
	mRhi = rhi;
}

size_t qHash(const QRhiGraphicsPipeline::TargetBlend& t) noexcept
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, t.enable);
	seed = hasher(seed, t.colorWrite);
	seed = hasher(seed, t.srcColor);
	seed = hasher(seed, t.srcAlpha);
	seed = hasher(seed, t.dstColor);
	seed = hasher(seed, t.dstAlpha);
	seed = hasher(seed, t.opColor);
	seed = hasher(seed, t.opAlpha);
	return seed;
}

size_t qHash(const QRhiGraphicsPipeline::StencilOpState& t) noexcept
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, t.compareOp);
	seed = hasher(seed, t.depthFailOp);
	seed = hasher(seed, t.failOp);
	seed = hasher(seed, t.passOp);
	return seed;
}

size_t QRGRhiResourcePool::hash(QRhiBuffer::Type type, QRhiBuffer::UsageFlags usage, quint32 size)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, type);
	seed = hasher(seed, usage);
	seed = hasher(seed, size);
	return seed;
}

size_t QRGRhiResourcePool::hash(QRhiTexture::Format format, const QSize& pixelSize, int sampleCount, QRhiTexture::Flags flags)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, format);
	seed = hasher(seed, pixelSize);
	seed = hasher(seed, sampleCount);
	seed = hasher(seed, flags);
	return seed;
}

size_t QRGRhiResourcePool::hash(QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, magFilter);
	seed = hasher(seed, minFilter);
	seed = hasher(seed, mipmapMode);
	seed = hasher(seed, addressU);
	seed = hasher(seed, addressV);
	seed = hasher(seed, addressW);
	return seed;
}

size_t QRGRhiResourcePool::hash(QVector<QRhiShaderResourceBinding> bindings)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, bindings);
	return seed;
}

size_t QRGRhiResourcePool::hash(QRhiRenderBuffer::Type type, const QSize& pixelSize, int sampleCount, QRhiRenderBuffer::Flags flags, QRhiTexture::Format backingFormatHint)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, type);
	seed = hasher(seed, pixelSize);
	seed = hasher(seed, sampleCount);
	seed = hasher(seed, flags);
	seed = hasher(seed, backingFormatHint);
	return seed;
}

size_t QRGRhiResourcePool::hash(const QRhiTextureRenderTargetDescription& desc, QRhiTextureRenderTarget::Flags flags)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, desc.colorAttachmentCount());
	for (int i = 0; i < desc.colorAttachmentCount(); i++) {
		const QRhiColorAttachment* colorAttach = desc.colorAttachmentAt(i);
		seed = hasher(seed, colorAttach->texture());
		seed = hasher(seed, colorAttach->renderBuffer());
		seed = hasher(seed, colorAttach->resolveTexture());
		seed = hasher(seed, colorAttach->layer());
		seed = hasher(seed, colorAttach->level());
		seed = hasher(seed, colorAttach->resolveLayer());
		seed = hasher(seed, colorAttach->resolveLevel());
	}
	seed = hasher(seed, desc.depthTexture());
	seed = hasher(seed, desc.depthStencilBuffer());
	seed = hasher(seed, flags);
	return seed;
}

size_t QRGRhiResourcePool::hash(const QRhiGraphicsPipelineState& state)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, state.flags);
	seed = hasher(seed, state.topology);
	seed = hasher(seed, state.cullMode);
	seed = hasher(seed, state.frontFace);
	seed = hasher(seed, state.targetBlends);
	seed = hasher(seed, state.depthTest);
	seed = hasher(seed, state.depthWrite);
	seed = hasher(seed, state.depthOp);
	seed = hasher(seed, state.stencilTest);
	seed = hasher(seed, state.stencilFront);
	seed = hasher(seed, state.stencilBack);
	seed = hasher(seed, state.stencilReadMask);
	seed = hasher(seed, state.stencilWriteMask);
	seed = hasher(seed, state.sampleCount);
	seed = hasher(seed, state.lineWidth);
	seed = hasher(seed, state.depthBias);
	seed = hasher(seed, state.slopeScaledDepthBias);
	seed = hasher(seed, state.patchControlPointCount);
	seed = hasher(seed, state.polygonMode);
	seed = hasher(seed, state.shaderStages);
	seed = hasher(seed, state.vertexInputLayout);
	seed = hasher(seed, state.shaderResourceBindings);
	seed = hasher(seed, state.renderPassDesc);

	return seed;
}

size_t QRGRhiResourcePool::hash(const QRhiComputePipelineState& state)
{
	size_t seed = 0;
	QtPrivate::QHashCombine hasher;
	seed = hasher(seed, state.shaderStage);
	seed = hasher(seed, state.flags);
	seed = hasher(seed, state.shaderResourceBindings);
	return seed;
}

QRhiBufferRef QRGRhiResourcePool::findOrNew(QRhiBuffer::Type type, QRhiBuffer::UsageFlags usage, quint32 size)
{
	size_t hashCode = hash(type, usage, size);
	for (auto it = mBufferPool.find(hashCode); it != mBufferPool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiBufferRef newRes(mRhi->newBuffer(type, usage, size));
	mBufferPool.insert(hashCode, newRes);
	mBufferToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiTextureRef QRGRhiResourcePool::findOrNew(QRhiTexture::Format format, const QSize& pixelSize, int sampleCount, QRhiTexture::Flags flags)
{
	size_t hashCode = hash(format, pixelSize, sampleCount, flags);
	for (auto it = mTexturePool.find(hashCode); it != mTexturePool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiTextureRef newRes(mRhi->newTexture(format, pixelSize, sampleCount, flags));
	mTexturePool.insert(hashCode, newRes);
	mTextureToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiSamplerRef QRGRhiResourcePool::findOrNew(QRhiSampler::Filter magFilter, QRhiSampler::Filter minFilter, QRhiSampler::Filter mipmapMode, QRhiSampler::AddressMode addressU, QRhiSampler::AddressMode addressV, QRhiSampler::AddressMode addressW)
{
	size_t hashCode = hash(magFilter, minFilter, mipmapMode, addressU, addressV, addressW);
	for (auto it = mSamplerPool.find(hashCode); it != mSamplerPool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiSamplerRef newRes(mRhi->newSampler(magFilter, minFilter, mipmapMode, addressU, addressV, addressW));
	mSamplerPool.insert(hashCode, newRes);
	mSamplerToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiShaderResourceBindingsRef QRGRhiResourcePool::findOrNew(QVector<QRhiShaderResourceBinding> bindings)
{
	size_t hashCode = hash(bindings);
	for (auto it = mBindingsPool.find(hashCode); it != mBindingsPool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiShaderResourceBindingsRef newRes(mRhi->newShaderResourceBindings());
	newRes->setBindings(bindings.begin(),bindings.end());
	mBindingsPool.insert(hashCode, newRes);
	mBindingsToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiRenderBufferRef QRGRhiResourcePool::findOrNew(QRhiRenderBuffer::Type type, const QSize& pixelSize, int sampleCount, QRhiRenderBuffer::Flags flags, QRhiTexture::Format backingFormatHint)
{
	size_t hashCode = hash(type,pixelSize, sampleCount, flags, backingFormatHint);
	for (auto it = mRenderBufferPool.find(hashCode); it != mRenderBufferPool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiRenderBufferRef newRes(mRhi->newRenderBuffer(type, pixelSize, sampleCount, flags, backingFormatHint));
	mRenderBufferPool.insert(hashCode, newRes);
	mRenderBufferToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiTextureRenderTargetRef QRGRhiResourcePool::findOrNew(const QRhiTextureRenderTargetDescription& desc, QRhiTextureRenderTarget::Flags flags)
{
	size_t hashCode = hash(desc, flags);
	for (auto it = mRenderTargetPool.find(hashCode); it != mRenderTargetPool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiTextureRenderTargetRef newRes(mRhi->newTextureRenderTarget(desc, flags));
	newRes->setRenderPassDescriptor((QRhiRenderPassDescriptor*)newRes.get());
	mRenderTargetPool.insert(hashCode, newRes);
	mRenderTargetToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiGraphicsPipelineRef QRGRhiResourcePool::findOrNew(const QRhiGraphicsPipelineState& state)
{
	size_t hashCode = hash(state);
	for (auto it = mGraphicsPipelinePool.find(hashCode); it != mGraphicsPipelinePool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiGraphicsPipelineRef newRes(mRhi->newGraphicsPipeline());
	state.assignTo(newRes.get());
	mGraphicsPipelinePool.insert(hashCode, newRes);
	mGraphicsPipelineToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

QRhiComputePipelineRef QRGRhiResourcePool::findOrNew(const QRhiComputePipelineState& state)
{
	size_t hashCode = hash(state);
	for (auto it = mComputePipelinePool.find(hashCode); it != mComputePipelinePool.end() && it.key() == hashCode; it++) {
		if (it.value().use_count() == 1) {
			return it.value();
		}
	}
	QRhiComputePipelineRef newRes(mRhi->newComputePipeline());
	state.assignTo(newRes.get());
	mComputePipelinePool.insert(hashCode, newRes);
	mComputePipelineToRecreate.push_back(newRes.get());
	mRhiHashMap[newRes.get()] = hashCode;
	return newRes;
}

void QRGRhiResourcePool::checkValidity(QRhiBufferRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(res->type(),res->usage(),res->size());
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mBufferToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiTextureRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(res->format(), res->pixelSize(), res->sampleCount(), res->flags());
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mTextureToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiSamplerRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(res->magFilter(), res->minFilter(), res->mipmapMode(), res->addressU(), res->addressV(), res->addressW());
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mSamplerToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiShaderResourceBindingsRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	QVector<QRhiShaderResourceBinding> bindings(res->cbeginBindings(), res->cendBindings());
	size_t currHash = hash(bindings);
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mBindingsToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiRenderBufferRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(res->type(),res->pixelSize(),res->sampleCount(),res->flags(),res->backingFormat());
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mRenderBufferToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiTextureRenderTargetRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(res->description(),res->flags());
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mRenderTargetToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiGraphicsPipelineRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(QRhiGraphicsPipelineState::createFrom(res.get()));
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mGraphicsPipelineToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::checkValidity(QRhiComputePipelineRef res)
{
	Q_ASSERT(res);
	size_t lastHash = mRhiHashMap.value(res.get());
	size_t currHash = hash(QRhiComputePipelineState::createFrom(res.get()));
	if (lastHash != currHash) {
		fixupResHash(res.get(), currHash);
		mComputePipelineToRecreate.push_back(res.get());
	}
}

void QRGRhiResourcePool::recreateBuffers()
{
	for (auto res : mBufferToRecreate) {
		Q_ASSERT(res);
		res->create();
	}
	mBufferToRecreate.clear();
}

void QRGRhiResourcePool::recreateTextures()
{
	for (auto res : mTextureToRecreate) {
		Q_ASSERT(res);
		res->create();
	}
	mTextureToRecreate.clear();
}

void QRGRhiResourcePool::recreateSamplers()
{
	for (auto res : mSamplerToRecreate) {
		Q_ASSERT(res);
		res->create();
	}
	mSamplerToRecreate.clear();
}

void QRGRhiResourcePool::recreateBindings()
{
	for (auto res : mBindingsToRecreate) {
		Q_ASSERT(res);
		res->create();
	}
	mBindingsToRecreate.clear();
}

void QRGRhiResourcePool::recreateRenderBuffers()
{
	for (auto res : mRenderBufferToRecreate) {
		Q_ASSERT(res);
		res->create();
	}
	mRenderBufferToRecreate.clear();
}

void QRGRhiResourcePool::recreateRenderTargets()
{
	for (auto res : mRenderTargetToRecreate) {
		Q_ASSERT(res);
		mRenderPassDescPool[res].reset(res->newCompatibleRenderPassDescriptor());
		if (res->renderPassDescriptor() == (QRhiRenderPassDescriptor*)res) {
			mRenderPassDescToRedirect.insert(res->renderPassDescriptor(), mRenderPassDescPool[res].get());
		}
		res->setRenderPassDescriptor(mRenderPassDescPool[res].get());
		res->create();
	}
	mRenderTargetToRecreate.clear();
}

void QRGRhiResourcePool::recreateGraphicsPipelines()
{
	for (auto res : mGraphicsPipelineToRecreate) {
		Q_ASSERT(res);
		auto redirectRenderPassDesc = mRenderPassDescToRedirect.value(res->renderPassDescriptor(), nullptr);
		if (redirectRenderPassDesc) {
			res->setRenderPassDescriptor(redirectRenderPassDesc);
			size_t currHash = hash(QRhiGraphicsPipelineState::createFrom(res));
			fixupResHash(res, currHash);
		}
		res->create();
	}
	mGraphicsPipelineToRecreate.clear();
}

void QRGRhiResourcePool::recreateComputePipelines()
{
	for (auto res : mComputePipelineToRecreate) {
		Q_ASSERT(res);
		res->create();
	}
	mComputePipelineToRecreate.clear();
	mRenderPassDescToRedirect.empty();
}

bool QRGRhiResourcePool::fixupResHash(QRhiResource* res, size_t newHash)
{
	Q_ASSERT(res);
	size_t oldHash = mRhiHashMap.value(res);
	if (res->resourceType() ==QRhiResource::Buffer) {
		for (auto it = mBufferPool.find(oldHash); it != mBufferPool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiBufferRef buffer = it.value();
					mBufferPool.remove(it.key(), it.value());
					mBufferPool.insert(newHash, buffer);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::Texture) {
		for (auto it = mTexturePool.find(oldHash); it != mTexturePool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiTextureRef value = it.value();
					mTexturePool.remove(it.key(), it.value());
					mTexturePool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::Sampler) {
		for (auto it = mSamplerPool.find(oldHash); it != mSamplerPool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiSamplerRef value = it.value();
					mSamplerPool.remove(it.key(), it.value());
					mSamplerPool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::ShaderResourceBindings) {
		for (auto it = mBindingsPool.find(oldHash); it != mBindingsPool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiShaderResourceBindingsRef value = it.value();
					mBindingsPool.remove(it.key(), it.value());
					mBindingsPool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::RenderBuffer) {
		for (auto it = mRenderBufferPool.find(oldHash); it != mRenderBufferPool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiRenderBufferRef value = it.value();
					mRenderBufferPool.remove(it.key(), it.value());
					mRenderBufferPool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::TextureRenderTarget) {
		for (auto it = mRenderTargetPool.find(oldHash); it != mRenderTargetPool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiTextureRenderTargetRef value = it.value();
					mRenderTargetPool.remove(it.key(), it.value());
					mRenderTargetPool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::GraphicsPipeline) {
		for (auto it = mGraphicsPipelinePool.find(oldHash); it != mGraphicsPipelinePool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				if (newHash != oldHash) {
					QRhiGraphicsPipelineRef value = it.value();
					mGraphicsPipelinePool.remove(it.key(), it.value());
					mGraphicsPipelinePool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	else if (res->resourceType() == QRhiResource::ComputePipeline) {
		for (auto it = mComputePipelinePool.find(oldHash); it != mComputePipelinePool.end() && it.key() == oldHash; it++) {
			if (it->get() == res) {
				size_t newHash = 0;
				if (newHash != oldHash) {
					QRhiComputePipelineRef value = it.value();
					mComputePipelinePool.remove(it.key(), it.value());
					mComputePipelinePool.insert(newHash, value);
					mRhiHashMap[res] = newHash;
					return true;
				}
			}
		}
	}
	return false;
}
