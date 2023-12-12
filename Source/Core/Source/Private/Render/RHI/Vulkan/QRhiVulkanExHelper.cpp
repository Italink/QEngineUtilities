#include "QRhiVulkanExHelper.h"
#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>
#include <qmath.h>
#include <QVulkanFunctions>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include "qrhivulkan_p_p.h"
#else
#include "private/qrhivulkan_p_p.h"
#endif

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_RECORDING_ENABLED 0
#define VMA_DEDICATED_ALLOCATION 0
#define VMA_DEBUG_LOG qDebug
#ifdef QT_DEBUG
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#endif
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-override")
#if defined(Q_CC_CLANG) && Q_CC_CLANG >= 1100
QT_WARNING_DISABLE_CLANG("-Wdeprecated-copy")
#endif
#define VMA_VULKAN_VERSION 1002000
#include "vk_mem_alloc.h"
QT_WARNING_POP

static QVulkanInstance* globalVulkanInstance;

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL wrap_vkGetInstanceProcAddr(VkInstance, const char *pName)
{
    return globalVulkanInstance->getInstanceProcAddr(pName);
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL wrap_vkGetDeviceProcAddr(VkDevice device, const char *pName)
{
    return globalVulkanInstance->functions()->vkGetDeviceProcAddr(device, pName);
}

static constexpr inline bool isDepthTextureFormat(QRhiTexture::Format format)
{
	switch (format) {
	case QRhiTexture::Format::D16:
	case QRhiTexture::Format::D24:
	case QRhiTexture::Format::D24S8:
	case QRhiTexture::Format::D32F:
		return true;

	default:
		return false;
	}
}

static constexpr inline VkImageAspectFlags aspectMaskForTextureFormat(QRhiTexture::Format format)
{
	return isDepthTextureFormat(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
}


struct QVkBufferEx : public QRhiBuffer {
	QVkBufferEx(QRhiImplementation* rhi, Type type, VkBufferUsageFlags usage, int size);
	~QVkBufferEx();
	void destroy() override;
	bool create() override;
	QRhiBuffer::NativeBuffer nativeBuffer() override;
	char* beginFullDynamicBufferUpdateForCurrentFrame() override;
	void endFullDynamicBufferUpdateForCurrentFrame() override;

	VkBuffer buffers[QVK_FRAMES_IN_FLIGHT];
	QVkAlloc allocations[QVK_FRAMES_IN_FLIGHT];
	struct DynamicUpdate {
		int offset;
		QRhiBufferData data;
	};
	QVarLengthArray<DynamicUpdate, 16> pendingDynamicUpdates[QVK_FRAMES_IN_FLIGHT];
	VkBuffer stagingBuffers[QVK_FRAMES_IN_FLIGHT];
	QVkAlloc stagingAllocations[QVK_FRAMES_IN_FLIGHT];
	struct UsageState {
		VkAccessFlags access = 0;
		VkPipelineStageFlags stage = 0;
	};
	UsageState usageState[QVK_FRAMES_IN_FLIGHT];
	int lastActiveFrameSlot = -1;
	uint generation = 0;
	friend class QRhiVulkan;
	VkBufferUsageFlags mUsage;
};

static inline VmaAllocation toVmaAllocation(QVkAlloc a) {
	return reinterpret_cast<VmaAllocation>(a);
}

static inline VmaAllocator toVmaAllocator(QVkAllocator a) {
	return reinterpret_cast<VmaAllocator>(a);
}

static inline QRhiBuffer::UsageFlags toQRhiufferUsage(VkBufferUsageFlags usage) {
	QRhiBuffer::UsageFlags u;
	if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
		u |= QRhiBuffer::VertexBuffer;
	else if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
		u |= QRhiBuffer::IndexBuffer;
	else if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		u |= QRhiBuffer::UniformBuffer;
	else if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
		u |= QRhiBuffer::StorageBuffer;
	return u;
}

QVkBufferEx::QVkBufferEx(QRhiImplementation* rhi, Type type, VkBufferUsageFlags usage, int size)
	: QRhiBuffer(rhi, type, toQRhiufferUsage(usage), size) {
	mUsage = usage;
	for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
		buffers[i] = stagingBuffers[i] = VK_NULL_HANDLE;
		allocations[i] = stagingAllocations[i] = nullptr;
	}
}

QVkBufferEx::~QVkBufferEx() {
	destroy();
}

void QVkBufferEx::destroy() {
	if (!buffers[0])
		return;

	QRhiVulkan::DeferredReleaseEntry e;
	e.type = QRhiVulkan::DeferredReleaseEntry::Buffer;
	e.lastActiveFrameSlot = lastActiveFrameSlot;

	for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
		e.buffer.buffers[i] = buffers[i];
		e.buffer.allocations[i] = allocations[i];
		e.buffer.stagingBuffers[i] = stagingBuffers[i];
		e.buffer.stagingAllocations[i] = stagingAllocations[i];

		buffers[i] = VK_NULL_HANDLE;
		allocations[i] = nullptr;
		stagingBuffers[i] = VK_NULL_HANDLE;
		stagingAllocations[i] = nullptr;
		pendingDynamicUpdates[i].clear();
	}

	QRHI_RES_RHI(QRhiVulkan);
	// destroy() implementations, unlike other functions, are expected to test
	// for m_rhi being null, to allow surviving in case one attempts to destroy
	// a (leaked) resource after the QRhi.
	if (rhiD) {
		for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
			vmaDestroyBuffer(toVmaAllocator(rhiD->allocator), e.buffer.buffers[i], toVmaAllocation(e.buffer.allocations[i]));
			vmaDestroyBuffer(toVmaAllocator(rhiD->allocator), e.buffer.stagingBuffers[i], toVmaAllocation(e.buffer.stagingAllocations[i]));
		}
		rhiD->unregisterResource(this);
	}
}

bool QVkBufferEx::create() {
	if (buffers[0])
		destroy();

	if (m_usage.testFlag(QRhiBuffer::StorageBuffer) && type() == Dynamic) {
		qWarning("StorageBuffer cannot be combined with Dynamic");
		return false;
	}

	const int nonZeroSize = m_size <= 0 ? 256 : m_size;

	VkBufferCreateInfo bufferInfo;
	memset(&bufferInfo, 0, sizeof(bufferInfo));
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = uint32_t(nonZeroSize);
	bufferInfo.usage = mUsage;

	VmaAllocationCreateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));

	if (type() == Dynamic) {
	#ifndef Q_OS_DARWIN // not for MoltenVK
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	#endif
		// host visible, frequent changes
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	}
	else {
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	}

	QRHI_RES_RHI(QRhiVulkan);
	VkResult err = VK_SUCCESS;
	for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
		buffers[i] = VK_NULL_HANDLE;
		allocations[i] = nullptr;
		usageState[i].access = usageState[i].stage = 0;
		if (i == 0 || type() == Dynamic) {
			VmaAllocation allocation;
			err = vmaCreateBuffer(toVmaAllocator(rhiD->allocator), &bufferInfo, &allocInfo, &buffers[i], &allocation, nullptr);
			if (err != VK_SUCCESS)
				break;
			allocations[i] = allocation;
		}
	}

	if (err != VK_SUCCESS) {
		qWarning("Failed to create buffer: %d", err);
		return false;
	}

	lastActiveFrameSlot = -1;
	generation += 1;
	rhiD->registerResource(this);
	return true;
}

void executeBufferHostWritesForSlot(QVkBuffer* bufD, int slot, QRhiVulkan* rhiD) {
	if (bufD->pendingDynamicUpdates[slot].isEmpty())
		return;

	//Q_ASSERT(bufD->type() == QRhiBuffer::Dynamic);
	void* p = nullptr;
	VmaAllocation a = toVmaAllocation(bufD->allocations[slot]);
	// The vmaMap/Unmap are basically a no-op when persistently mapped since it
	// refcounts; this is great because we don't need to care if the allocation
	// was created as persistently mapped or not.
	VkResult err = vmaMapMemory(toVmaAllocator(rhiD->allocator), a, &p);
	if (err != VK_SUCCESS) {
		qWarning("Failed to map buffer: %d", err);
		return;
	}
	int changeBegin = -1;
	int changeEnd = -1;
	for (const QVkBuffer::DynamicUpdate& u : qAsConst(bufD->pendingDynamicUpdates[slot])) {
		memcpy(static_cast<char*>(p) + u.offset, u.data.constData(), size_t(u.data.size()));
		if (changeBegin == -1 || u.offset < changeBegin)
			changeBegin = u.offset;
		if (changeEnd == -1 || u.offset + u.data.size() > changeEnd)
			changeEnd = u.offset + u.data.size();
	}
	vmaUnmapMemory(toVmaAllocator(rhiD->allocator), a);
	if (changeBegin >= 0)
		vmaFlushAllocation(toVmaAllocator(rhiD->allocator), a, VkDeviceSize(changeBegin), VkDeviceSize(changeEnd - changeBegin));

	bufD->pendingDynamicUpdates[slot].clear();
}

QRhiBuffer::NativeBuffer QVkBufferEx::nativeBuffer() {
	if (type() == Dynamic) {
		QRHI_RES_RHI(QRhiVulkan);
		NativeBuffer b;
		Q_ASSERT(sizeof(b.objects) / sizeof(b.objects[0]) >= size_t(QVK_FRAMES_IN_FLIGHT));
		for (int i = 0; i < QVK_FRAMES_IN_FLIGHT; ++i) {
			executeBufferHostWritesForSlot((QVkBuffer*)this, i,rhiD);
			b.objects[i] = &buffers[i];
		}
		b.slotCount = QVK_FRAMES_IN_FLIGHT;
		return b;
	}
	return { { &buffers[0] }, 1 };
}

char* QVkBufferEx::beginFullDynamicBufferUpdateForCurrentFrame() {
	Q_ASSERT(type() == Dynamic);
	QRHI_RES_RHI(QRhiVulkan);
	Q_ASSERT(rhiD->inFrame);
	const int slot = rhiD->currentFrameSlot;
	void* p = nullptr;
	VmaAllocation a = toVmaAllocation(allocations[slot]);
	VkResult err = vmaMapMemory(toVmaAllocator(rhiD->allocator), a, &p);
	if (err != VK_SUCCESS) {
		qWarning("Failed to map buffer: %d", err);
		return nullptr;
	}
	return static_cast<char*>(p);
}

void QVkBufferEx::endFullDynamicBufferUpdateForCurrentFrame() {
	QRHI_RES_RHI(QRhiVulkan);
	const int slot = rhiD->currentFrameSlot;
	VmaAllocation a = toVmaAllocation(allocations[slot]);
	vmaUnmapMemory(toVmaAllocator(rhiD->allocator), a);
	vmaFlushAllocation(toVmaAllocator(rhiD->allocator), a, 0, m_size);
}

QVulkanInstance* QRhiVulkanExHelper::instance()
{
	if (globalVulkanInstance)
		return globalVulkanInstance;
	globalVulkanInstance = new QVulkanInstance;
	const QVersionNumber supportedVersion = globalVulkanInstance->supportedApiVersion();
	if (supportedVersion >= QVersionNumber(1, 3))
		globalVulkanInstance->setApiVersion(QVersionNumber(1, 3));
	else if (supportedVersion >= QVersionNumber(1, 2))
		globalVulkanInstance->setApiVersion(QVersionNumber(1, 2));
	else if (supportedVersion >= QVersionNumber(1, 1))
		globalVulkanInstance->setApiVersion(QVersionNumber(1, 1));

	qDebug()<< globalVulkanInstance->supportedApiVersion()
		<< globalVulkanInstance->supportedExtensions()
		<< globalVulkanInstance->supportedLayers();

	globalVulkanInstance->setLayers({ "VK_LAYER_KHRONOS_validation"/*,"VK_LAYER_TENCENT_wegame_cross_overlay","VK_LAYER_LUNARG_screenshot","VK_LAYER_NV_optimus","VK_LAYER_LUNARG_gfxreconstruct","VK_LAYER_LUNARG_monitor" */});
	globalVulkanInstance->setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
	if (!globalVulkanInstance->create()) {
		qWarning("Failed to create Vulkan instance");
		delete globalVulkanInstance;
		globalVulkanInstance = nullptr;
	}
	return globalVulkanInstance;
}

QRhiBuffer* QRhiVulkanExHelper::newVkBuffer(QRhi* inRhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size) {
	QRhiImplementation* ptr = (*(QRhiImplementation**)inRhi);
	return new QVkBufferEx(ptr, type, flags, size);
}

void QRhiVulkanExHelper::updateShaderResourceBindings(QVulkanDeviceFunctions* df,VkDevice dev, QRhiVulkan* rhi, QRhiShaderResourceBindings* srb, int descSetIdx /*= -1*/)
{
	QVkShaderResourceBindings* srbD = QRHI_RES(QVkShaderResourceBindings, srb);

	QVarLengthArray<VkDescriptorBufferInfo, 8> bufferInfos;
	using ArrayOfImageDesc = QVarLengthArray<VkDescriptorImageInfo, 8>;
	QVarLengthArray<ArrayOfImageDesc, 8> imageInfos;
	QVarLengthArray<VkWriteDescriptorSet, 12> writeInfos;
	QVarLengthArray<QPair<int, int>, 12> infoIndices;

	const bool updateAll = descSetIdx < 0;
	int frameSlot = updateAll ? 0 : descSetIdx;
	while (frameSlot < (updateAll ? QVK_FRAMES_IN_FLIGHT : descSetIdx + 1)) {
		for (int i = 0, ie = srbD->sortedBindings.size(); i != ie; ++i) {
			const QRhiShaderResourceBinding::Data* b = rhi->shaderResourceBindingData(srbD->sortedBindings.at(i));
			QVkShaderResourceBindings::BoundResourceData& bd(srbD->boundResourceData[frameSlot][i]);

			VkWriteDescriptorSet writeInfo = {};
			writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeInfo.dstSet = srbD->descSets[frameSlot];
			writeInfo.dstBinding = uint32_t(b->binding);
			writeInfo.descriptorCount = 1;

			int bufferInfoIndex = -1;
			int imageInfoIndex = -1;

			switch (b->type) {
			case QRhiShaderResourceBinding::UniformBuffer:
			{
				writeInfo.descriptorType = b->u.ubuf.hasDynamicOffset ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
					: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				QRhiBuffer* buf = b->u.ubuf.buf;
				QVkBuffer* bufD = QRHI_RES(QVkBuffer, buf);
				bd.ubuf.id = bufD->globalResourceId();
				bd.ubuf.generation = bufD->generation;
				VkDescriptorBufferInfo bufInfo;
				bufInfo.buffer = bufD->type() == QRhiBuffer::Dynamic ? bufD->buffers[frameSlot] : bufD->buffers[0];
				bufInfo.offset = b->u.ubuf.offset;
				bufInfo.range = b->u.ubuf.maybeSize ? b->u.ubuf.maybeSize : bufD->size();
				bufferInfoIndex = bufferInfos.size();
				bufferInfos.append(bufInfo);
			}
			break;
			case QRhiShaderResourceBinding::SampledTexture:
			{
				const QRhiShaderResourceBinding::Data::TextureAndOrSamplerData* data = &b->u.stex;
				writeInfo.descriptorCount = data->count; // arrays of combined image samplers are supported
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				ArrayOfImageDesc imageInfo(data->count);
				for (int elem = 0; elem < data->count; ++elem) {
					QVkTexture* texD = QRHI_RES(QVkTexture, data->texSamplers[elem].tex);
					QVkSampler* samplerD = QRHI_RES(QVkSampler, data->texSamplers[elem].sampler);
					bd.stex.d[elem].texId = texD->globalResourceId();
					bd.stex.d[elem].texGeneration = texD->generation;
					bd.stex.d[elem].samplerId = samplerD->globalResourceId();
					bd.stex.d[elem].samplerGeneration = samplerD->generation;
					imageInfo[elem].sampler = samplerD->sampler;
					imageInfo[elem].imageView = texD->imageView;
					imageInfo[elem].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				bd.stex.count = data->count;
				imageInfoIndex = imageInfos.size();
				imageInfos.append(imageInfo);
			}
			break;
			case QRhiShaderResourceBinding::Texture:
			{
				const QRhiShaderResourceBinding::Data::TextureAndOrSamplerData* data = &b->u.stex;
				writeInfo.descriptorCount = data->count; // arrays of (separate) images are supported
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				ArrayOfImageDesc imageInfo(data->count);
				for (int elem = 0; elem < data->count; ++elem) {
					QVkTexture* texD = QRHI_RES(QVkTexture, data->texSamplers[elem].tex);
					bd.stex.d[elem].texId = texD->globalResourceId();
					bd.stex.d[elem].texGeneration = texD->generation;
					bd.stex.d[elem].samplerId = 0;
					bd.stex.d[elem].samplerGeneration = 0;
					imageInfo[elem].sampler = VK_NULL_HANDLE;
					imageInfo[elem].imageView = texD->imageView;
					imageInfo[elem].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				bd.stex.count = data->count;
				imageInfoIndex = imageInfos.size();
				imageInfos.append(imageInfo);
			}
			break;
			case QRhiShaderResourceBinding::Sampler:
			{
				QVkSampler* samplerD = QRHI_RES(QVkSampler, b->u.stex.texSamplers[0].sampler);
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
				bd.stex.d[0].texId = 0;
				bd.stex.d[0].texGeneration = 0;
				bd.stex.d[0].samplerId = samplerD->globalResourceId();
				bd.stex.d[0].samplerGeneration = samplerD->generation;
				ArrayOfImageDesc imageInfo(1);
				imageInfo[0].sampler = samplerD->sampler;
				imageInfo[0].imageView = VK_NULL_HANDLE;
				imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
				imageInfoIndex = imageInfos.size();
				imageInfos.append(imageInfo);
			}
			break;
			case QRhiShaderResourceBinding::ImageLoad:
			case QRhiShaderResourceBinding::ImageStore:
			case QRhiShaderResourceBinding::ImageLoadStore:
			{
				QVkTexture* texD = QRHI_RES(QVkTexture, b->u.simage.tex);
				VkImageView view = imageViewForLevel(texD,b->u.simage.level);
				if (view) {
					writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					bd.simage.id = texD->globalResourceId();
					bd.simage.generation = texD->generation;
					ArrayOfImageDesc imageInfo(1);
					imageInfo[0].sampler = VK_NULL_HANDLE;
					imageInfo[0].imageView = view;
					imageInfo[0].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					imageInfoIndex = imageInfos.size();
					imageInfos.append(imageInfo);
				}
			}
			break;
			case QRhiShaderResourceBinding::BufferLoad:
			case QRhiShaderResourceBinding::BufferStore:
			case QRhiShaderResourceBinding::BufferLoadStore:
			{
				QVkBuffer* bufD = QRHI_RES(QVkBuffer, b->u.sbuf.buf);
				writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				bd.sbuf.id = bufD->globalResourceId();
				bd.sbuf.generation = bufD->generation;
				VkDescriptorBufferInfo bufInfo;
				bufInfo.buffer = bufD->type() == QRhiBuffer::Dynamic ? bufD->buffers[frameSlot] : bufD->buffers[0];
				bufInfo.offset = b->u.ubuf.offset;
				bufInfo.range = b->u.ubuf.maybeSize ? b->u.ubuf.maybeSize : bufD->size();
				bufferInfoIndex = bufferInfos.size();
				bufferInfos.append(bufInfo);
			}
			break;
			default:
				continue;
			}

			writeInfos.append(writeInfo);
			infoIndices.append({ bufferInfoIndex, imageInfoIndex });
		}
		++frameSlot;
	}

	for (int i = 0, writeInfoCount = writeInfos.size(); i < writeInfoCount; ++i) {
		const int bufferInfoIndex = infoIndices[i].first;
		const int imageInfoIndex = infoIndices[i].second;
		if (bufferInfoIndex >= 0)
			writeInfos[i].pBufferInfo = &bufferInfos[bufferInfoIndex];
		else if (imageInfoIndex >= 0)
			writeInfos[i].pImageInfo = imageInfos[imageInfoIndex].constData();
	}

	df->vkUpdateDescriptorSets(dev, uint32_t(writeInfos.size()), writeInfos.constData(), 0, nullptr);
}

VkImageView QRhiVulkanExHelper::imageViewForLevel(QVkTexture* texture, int level)
{
	Q_ASSERT(level >= 0 && level < int(texture->mipLevelCount));
	if (texture->perLevelImageViews[level] != VK_NULL_HANDLE)
		return texture->perLevelImageViews[level];

	const VkImageAspectFlags aspectMask = aspectMaskForTextureFormat(texture->format());
	const bool isCube = texture->flags().testFlag(QRhiTexture::CubeMap);
	const bool isArray = texture->flags().testFlag(QRhiTexture::TextureArray);
	const bool is3D = texture->flags().testFlag(QRhiTexture::ThreeDimensional);
	const bool is1D = texture->flags().testFlag(QRhiTexture::OneDimensional);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = texture->image;
	viewInfo.viewType = isCube
		? VK_IMAGE_VIEW_TYPE_CUBE
		: (is3D ? VK_IMAGE_VIEW_TYPE_3D
			: (is1D ? (isArray ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D)
				: (isArray ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D)));
	viewInfo.format = texture->vkformat;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewInfo.subresourceRange.aspectMask = aspectMask;
	viewInfo.subresourceRange.baseMipLevel = uint32_t(level);
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = isCube ? 6 : (isArray ? qMax(0, texture->arraySize()) : 1);

	VkImageView v = VK_NULL_HANDLE;


	QRhiVulkan* rhiD = *(QRhiVulkan**)(texture->rhi());
	VkResult err = rhiD->df->vkCreateImageView(rhiD->dev, &viewInfo, nullptr, &v);
	if (err != VK_SUCCESS) {
		qWarning("Failed to create image view: %d", err);
		return VK_NULL_HANDLE;
	}

	texture->perLevelImageViews[level] = v;
	return v;
}

void QRhiVulkanExHelper::setShaderResources(QRhiResource *pipeline, QRhiCommandBuffer* cb, QRhiShaderResourceBindings* srb, int dynamicOffsetCount, const QRhiCommandBuffer::DynamicOffset* dynamicOffsets)
{
	QRhiVulkan* rhi = *(QRhiVulkan**)(pipeline->rhi());
	QVkCommandBuffer* cbD = QRHI_RES(QVkCommandBuffer, cb);
	QVkGraphicsPipeline* gfxPsD = pipeline->resourceType() == QRhiResource::Type::GraphicsPipeline ? QRHI_RES(QVkGraphicsPipeline, pipeline) : nullptr;
	QVkComputePipeline* compPsD = pipeline->resourceType() == QRhiResource::Type::ComputePipeline ? QRHI_RES(QVkComputePipeline, pipeline) : nullptr;

	if (!srb) {
		if (gfxPsD)
			srb = gfxPsD->shaderResourceBindings();
		else
			srb = compPsD->shaderResourceBindings();
	}

	QVkShaderResourceBindings* srbD = QRHI_RES(QVkShaderResourceBindings, srb);
	const int descSetIdx = srbD->hasSlottedResource ? rhi->currentFrameSlot : 0;
	auto& descSetBd(srbD->boundResourceData[descSetIdx]);
	bool rewriteDescSet = false;

	// Do host writes and mark referenced shader resources as in-use.
	// Also prepare to ensure the descriptor set we are going to bind refers to up-to-date Vk objects.
	for (int i = 0, ie = srbD->sortedBindings.size(); i != ie; ++i) {
		const QRhiShaderResourceBinding::Data* b = rhi->shaderResourceBindingData(srbD->sortedBindings[i]);
		QVkShaderResourceBindings::BoundResourceData& bd(descSetBd[i]);
		switch (b->type) {
		case QRhiShaderResourceBinding::UniformBuffer:
		{
			QVkBuffer* bufD = QRHI_RES(QVkBuffer, b->u.ubuf.buf);
			Q_ASSERT(bufD->usage().testFlag(QRhiBuffer::UniformBuffer));

			if (bufD->type() == QRhiBuffer::Dynamic)
				executeBufferHostWritesForSlot(bufD, rhi->currentFrameSlot,rhi);

			bufD->lastActiveFrameSlot = rhi->currentFrameSlot;

			// Check both the "local" id (the generation counter) and the
			// global id. The latter is relevant when a newly allocated
			// QRhiResource ends up with the same pointer as a previous one.
			// (and that previous one could have been in an srb...)
			if (bufD->generation != bd.ubuf.generation || bufD->globalResourceId() != bd.ubuf.id) {
				rewriteDescSet = true;
				bd.ubuf.id = bufD->globalResourceId();
				bd.ubuf.generation = bufD->generation;
			}
		}
		break;
		case QRhiShaderResourceBinding::SampledTexture:
		case QRhiShaderResourceBinding::Texture:
		case QRhiShaderResourceBinding::Sampler:
		{
			const QRhiShaderResourceBinding::Data::TextureAndOrSamplerData* data = &b->u.stex;
			if (bd.stex.count != data->count) {
				bd.stex.count = data->count;
				rewriteDescSet = true;
			}
			for (int elem = 0; elem < data->count; ++elem) {
				QVkTexture* texD = QRHI_RES(QVkTexture, data->texSamplers[elem].tex);
				QVkSampler* samplerD = QRHI_RES(QVkSampler, data->texSamplers[elem].sampler);
				// We use the same code path for both combined and separate
				// images and samplers, so tex or sampler (but not both) can be
				// null here.
				Q_ASSERT(texD || samplerD);
				if (texD) {
					texD->lastActiveFrameSlot = rhi->currentFrameSlot;
				}
				if (samplerD)
					samplerD->lastActiveFrameSlot = rhi->currentFrameSlot;
				const quint64 texId = texD ? texD->globalResourceId() : 0;
				const uint texGen = texD ? texD->generation : 0;
				const quint64 samplerId = samplerD ? samplerD->globalResourceId() : 0;
				const uint samplerGen = samplerD ? samplerD->generation : 0;
				if (texGen != bd.stex.d[elem].texGeneration
					|| texId != bd.stex.d[elem].texId
					|| samplerGen != bd.stex.d[elem].samplerGeneration
					|| samplerId != bd.stex.d[elem].samplerId)
				{
					rewriteDescSet = true;
					bd.stex.d[elem].texId = texId;
					bd.stex.d[elem].texGeneration = texGen;
					bd.stex.d[elem].samplerId = samplerId;
					bd.stex.d[elem].samplerGeneration = samplerGen;
				}
			}
		}
		break;
		case QRhiShaderResourceBinding::ImageLoad:
		case QRhiShaderResourceBinding::ImageStore:
		case QRhiShaderResourceBinding::ImageLoadStore:
		{
			QVkTexture* texD = QRHI_RES(QVkTexture, b->u.simage.tex);
			Q_ASSERT(texD->flags().testFlag(QRhiTexture::UsedWithLoadStore));
			texD->lastActiveFrameSlot = rhi->currentFrameSlot;
			QRhiPassResourceTracker::TextureAccess access;
			if (b->type == QRhiShaderResourceBinding::ImageLoad)
				access = QRhiPassResourceTracker::TexStorageLoad;
			else if (b->type == QRhiShaderResourceBinding::ImageStore)
				access = QRhiPassResourceTracker::TexStorageStore;
			else
				access = QRhiPassResourceTracker::TexStorageLoadStore;

			if (texD->generation != bd.simage.generation || texD->globalResourceId() != bd.simage.id) {
				rewriteDescSet = true;
				bd.simage.id = texD->globalResourceId();
				bd.simage.generation = texD->generation;
			}
		}
		break;
		case QRhiShaderResourceBinding::BufferLoad:
		case QRhiShaderResourceBinding::BufferStore:
		case QRhiShaderResourceBinding::BufferLoadStore:
		{
			QVkBuffer* bufD = QRHI_RES(QVkBuffer, b->u.sbuf.buf);
			Q_ASSERT(bufD->usage().testFlag(QRhiBuffer::StorageBuffer));

			if (bufD->type() == QRhiBuffer::Dynamic)
				executeBufferHostWritesForSlot(bufD, rhi->currentFrameSlot,rhi);

			bufD->lastActiveFrameSlot = rhi->currentFrameSlot;
			QRhiPassResourceTracker::BufferAccess access;
			if (b->type == QRhiShaderResourceBinding::BufferLoad)
				access = QRhiPassResourceTracker::BufStorageLoad;
			else if (b->type == QRhiShaderResourceBinding::BufferStore)
				access = QRhiPassResourceTracker::BufStorageStore;
			else
				access = QRhiPassResourceTracker::BufStorageLoadStore;

			if (bufD->generation != bd.sbuf.generation || bufD->globalResourceId() != bd.sbuf.id) {
				rewriteDescSet = true;
				bd.sbuf.id = bufD->globalResourceId();
				bd.sbuf.generation = bufD->generation;
			}
		}
		break;
		default:
			Q_UNREACHABLE();
			break;
		}
	}

	// write descriptor sets, if needed
	if (rewriteDescSet)
		updateShaderResourceBindings(rhi->df,rhi->dev,rhi,srb, descSetIdx);

	// make sure the descriptors for the correct slot will get bound.
	// also, dynamic offsets always need a bind.
	const bool forceRebind = (srbD->hasSlottedResource && cbD->currentDescSetSlot != descSetIdx) || srbD->hasDynamicOffset;

	const bool srbChanged = gfxPsD ? (cbD->currentGraphicsSrb != srb) : (cbD->currentComputeSrb != srb);

	if (forceRebind || rewriteDescSet || srbChanged || cbD->currentSrbGeneration != srbD->generation) {
		QVarLengthArray<uint32_t, 4> dynOfs;
		if (srbD->hasDynamicOffset) {
			// Filling out dynOfs based on the sorted bindings is important
			// because dynOfs has to be ordered based on the binding numbers,
			// and neither srb nor dynamicOffsets has any such ordering
			// requirement.
			for (const QRhiShaderResourceBinding& binding : std::as_const(srbD->sortedBindings)) {
				const QRhiShaderResourceBinding::Data* b = rhi->shaderResourceBindingData(binding);
				if (b->type == QRhiShaderResourceBinding::UniformBuffer && b->u.ubuf.hasDynamicOffset) {
					uint32_t offset = 0;
					for (int i = 0; i < dynamicOffsetCount; ++i) {
						const QRhiCommandBuffer::DynamicOffset& bindingOffsetPair(dynamicOffsets[i]);
						if (bindingOffsetPair.first == b->binding) {
							offset = bindingOffsetPair.second;
							break;
						}
					}
					dynOfs.append(offset); // use 0 if dynamicOffsets did not contain this binding
				}
			}
		}

		rhi->df->vkCmdBindDescriptorSets(cbD->cb,
			gfxPsD ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE,
			gfxPsD ? gfxPsD->layout : compPsD->layout,
			0, 1, & srbD->descSets[descSetIdx],
			uint32_t(dynOfs.size()),
			dynOfs.size() ? dynOfs.constData() : nullptr);
		

		if (gfxPsD) {
			cbD->currentGraphicsSrb = srb;
			cbD->currentComputeSrb = nullptr;
		}
		else {
			cbD->currentGraphicsSrb = nullptr;
			cbD->currentComputeSrb = srb;
		}
		cbD->currentSrbGeneration = srbD->generation;
		cbD->currentDescSetSlot = descSetIdx;
	}

	srbD->lastActiveFrameSlot = rhi->currentFrameSlot;
}

QRhiVulkanNativeHandles  QRhiVulkanExHelper::createVulkanNativeHandles(const QRhiVulkanInitParams& params) {
	QRhiVulkanNativeHandles handles;
	auto& physDev = handles.physDev;
	auto& dev = handles.dev;
	auto& gfxQueueFamilyIdx = handles.gfxQueueFamilyIdx;
	auto& gfxQueueIdx = handles.gfxQueueIdx;
	auto& gfxQueue = handles.gfxQueue;
	auto& vmemAllocator = handles.vmemAllocator;
	auto& inst = params.inst;
	auto& maybeWindow = params.window;
	auto& requestedDeviceExtensions = params.deviceExtensions;
	auto f = inst->functions();

	globalVulkanInstance = params.inst;

	VkPhysicalDeviceProperties physDevProperties;
	uint32_t physDevCount = 0;
	f->vkEnumeratePhysicalDevices(inst->vkInstance(), &physDevCount, nullptr);
	if (!physDevCount) {
		qWarning("No physical devices");
	}
	QVarLengthArray<VkPhysicalDevice, 4> physDevs(physDevCount);
	VkResult err = f->vkEnumeratePhysicalDevices(inst->vkInstance(), &physDevCount, physDevs.data());
	if (err != VK_SUCCESS || !physDevCount) {
		qWarning("Failed to enumerate physical devices: %d", err);
	}

	int physDevIndex = -1;
	int requestedPhysDevIndex = -1;
	if (qEnvironmentVariableIsSet("QT_VK_PHYSICAL_DEVICE_INDEX"))
		requestedPhysDevIndex = qEnvironmentVariableIntValue("QT_VK_PHYSICAL_DEVICE_INDEX");

	for (int i = 0; i < int(physDevCount); ++i) {
		f->vkGetPhysicalDeviceProperties(physDevs[i], &physDevProperties);
		if (physDevIndex < 0 && (requestedPhysDevIndex < 0 || requestedPhysDevIndex == int(i))) {
			physDevIndex = i;
		}
	}

	if (physDevIndex < 0) {
		qWarning("No matching physical device");
	}
	physDev = physDevs[physDevIndex];
	f->vkGetPhysicalDeviceProperties(physDev, &physDevProperties);

	std::optional<uint32_t> gfxQueueFamilyIdxOpt;
	std::optional<uint32_t> computelessGfxQueueCandidateIdxOpt;
	QList<VkQueueFamilyProperties> queueFamilyProps;

	auto queryQueueFamilyProps = [&] {
		uint32_t queueCount = 0;
		f->vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, nullptr);
		queueFamilyProps.resize(int(queueCount));
		f->vkGetPhysicalDeviceQueueFamilyProperties(physDev, &queueCount, queueFamilyProps.data());
	};
	queryQueueFamilyProps();
	const uint32_t queueFamilyCount = uint32_t(queueFamilyProps.size());
	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		if (!gfxQueueFamilyIdxOpt.has_value()
			&& (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			&& (!maybeWindow || inst->supportsPresent(physDev, i, maybeWindow))) {
			if (queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
				gfxQueueFamilyIdxOpt = i;
			else if (!computelessGfxQueueCandidateIdxOpt.has_value())
				computelessGfxQueueCandidateIdxOpt = i;
		}
	}
	if (gfxQueueFamilyIdxOpt.has_value()) {
		gfxQueueFamilyIdx = gfxQueueFamilyIdxOpt.value();
	}
	else {
		if (computelessGfxQueueCandidateIdxOpt.has_value()) {
			gfxQueueFamilyIdx = computelessGfxQueueCandidateIdxOpt.value();
		}
		else {
			qWarning("No graphics (or no graphics+present) queue family found");
		}
	}

	VkDeviceQueueCreateInfo queueInfo = {};
	const float prio[] = { 0 };
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = gfxQueueFamilyIdx;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = prio;

	QList<const char*> devLayers;
	if (inst->layers().contains("VK_LAYER_KHRONOS_validation"))
		devLayers.append("VK_LAYER_KHRONOS_validation");

	QVulkanInfoVector<QVulkanExtension> devExts;
	uint32_t devExtCount = 0;
	f->vkEnumerateDeviceExtensionProperties(physDev, nullptr, &devExtCount, nullptr);
	if (devExtCount) {
		QList<VkExtensionProperties> extProps(devExtCount);
		f->vkEnumerateDeviceExtensionProperties(physDev, nullptr, &devExtCount, extProps.data());
		for (const VkExtensionProperties& p : std::as_const(extProps))
			devExts.append({ p.extensionName, p.specVersion });
	}

	QList<const char*> requestedDevExts;
	requestedDevExts.append("VK_KHR_swapchain");

	const bool hasPhysDevProp2 = inst->extensions().contains(QByteArrayLiteral("VK_KHR_get_physical_device_properties2"));

	if (devExts.contains(QByteArrayLiteral("VK_KHR_portability_subset"))) {
		if (hasPhysDevProp2) {
			requestedDevExts.append("VK_KHR_portability_subset");
		}
		else {
			qWarning("VK_KHR_portability_subset should be enabled on the device "
				"but the instance does not have VK_KHR_get_physical_device_properties2 enabled. "
				"Expect problems.");
		}
	}
	if (devExts.contains(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME)) {
		if (hasPhysDevProp2) {
			requestedDevExts.append(VK_EXT_VERTEX_ATTRIBUTE_DIVISOR_EXTENSION_NAME);
		}
	}

	for (const QByteArray& ext : requestedDeviceExtensions) {
		if (!ext.isEmpty() && !requestedDevExts.contains(ext)) {
			if (devExts.contains(ext)) {
				requestedDevExts.append(ext.constData());
			}
			else {
				qWarning("Device extension %s requested in QRhiVulkanInitParams is not supported",
					ext.constData());
			}
		}
	}

	QByteArrayList envExtList = qgetenv("QT_VULKAN_DEVICE_EXTENSIONS").split(';');
	for (const QByteArray& ext : envExtList) {
		if (!ext.isEmpty() && !requestedDevExts.contains(ext)) {
			if (devExts.contains(ext)) {
				requestedDevExts.append(ext.constData());
			}
			else {
				qWarning("Device extension %s requested in QT_VULKAN_DEVICE_EXTENSIONS is not supported",
					ext.constData());
			}
		}
	}

	VkDeviceCreateInfo devInfo = {};
	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &queueInfo;
	//devInfo.enabledLayerCount = uint32_t(devLayers.size());
	//devInfo.ppEnabledLayerNames = devLayers.constData();
	devInfo.enabledExtensionCount = uint32_t(requestedDevExts.size());
	devInfo.ppEnabledExtensionNames = requestedDevExts.constData();

#ifdef VK_VERSION_1_2 // Vulkan11Features is only in Vulkan 1.2
	VkPhysicalDeviceFeatures2 physDevFeatures2 = {};
	physDevFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

	VkPhysicalDeviceVulkan11Features features11 = {};
	features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	VkPhysicalDeviceVulkan12Features features12 = {};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
#ifdef VK_VERSION_1_3
	VkPhysicalDeviceVulkan13Features features13 = {};
	features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
#endif
	if (inst->apiVersion() >= QVersionNumber(1, 2)) {
		physDevFeatures2.pNext = &features11;
		features11.pNext = &features12;
	#ifdef VK_VERSION_1_3
		if (inst->apiVersion() >= QVersionNumber(1, 3))
			features12.pNext = &features13;
	#endif
		f->vkGetPhysicalDeviceFeatures2(physDev, &physDevFeatures2);

		physDevFeatures2.features.robustBufferAccess = VK_FALSE;
	#ifdef VK_VERSION_1_3
		features13.robustImageAccess = VK_FALSE;
	#endif

		devInfo.pNext = &physDevFeatures2;
	}
#endif // VK_VERSION_1_2

	VkPhysicalDeviceFeatures features;
	if (!devInfo.pNext) {
		features.robustBufferAccess = VK_FALSE;
		devInfo.pEnabledFeatures = &features;
	}

	err = f->vkCreateDevice(physDev, &devInfo, nullptr, &dev);
	if (err != VK_SUCCESS) {
		qWarning("Failed to create device: %d", err);
	}

	VmaVulkanFunctions funcs = {};
	funcs.vkGetInstanceProcAddr = wrap_vkGetInstanceProcAddr;
	funcs.vkGetDeviceProcAddr = wrap_vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocatorInfo = {};
	// A QRhi is supposed to be used from one single thread only. Disable
	// the allocator's own mutexes. This gives a performance boost.
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	allocatorInfo.physicalDevice = physDev;
	allocatorInfo.device = dev;
	allocatorInfo.pVulkanFunctions = &funcs;
	allocatorInfo.instance = inst->vkInstance();
	const QVersionNumber apiVer = inst->apiVersion();
	if (!apiVer.isNull()) {
		allocatorInfo.vulkanApiVersion = VK_MAKE_VERSION(apiVer.majorVersion(),
			apiVer.minorVersion(),
			apiVer.microVersion());
	}
	VmaAllocator vmaallocator;
	err = vmaCreateAllocator(&allocatorInfo, &vmaallocator);
	if (err != VK_SUCCESS) {
		qWarning("Failed to create allocator: %d", err);
	}
	//handles.vmemAllocator = vmaallocator;
	return handles;
}

void QRhiVulkanExHelper::destroyVulkanNativeHandles(const QRhiVulkanNativeHandles& handles) {
	if (handles.vmemAllocator) {
		vmaDestroyAllocator(toVmaAllocator(handles.vmemAllocator));
	}
	if (handles.dev) {
		auto df = globalVulkanInstance->deviceFunctions(handles.dev);
		df->vkDestroyDevice(handles.dev, nullptr);
		globalVulkanInstance->resetDeviceFunctions(handles.dev);
	}
}

