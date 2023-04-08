#include "QRhiVulkanExHelper.h"
#include "private/qrhi_p_p.h"
#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>

#include "private/qrhivulkanext_p.h"
#include "private/qvulkandefaultinstance_p.h"

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
		rhiD->releaseQueue.append(e);
		rhiD->unregisterResource(this);
	}
}

bool QVkBufferEx::create() {
	if (buffers[0])
		destroy();

	if (m_usage.testFlag(QRhiBuffer::StorageBuffer) && m_type == Dynamic) {
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

	if (m_type == Dynamic) {
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
		if (i == 0 || m_type == Dynamic) {
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

	//Q_ASSERT(bufD->m_type == QRhiBuffer::Dynamic);
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
	if (m_type == Dynamic) {
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
	Q_ASSERT(m_type == Dynamic);
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

QRhiBuffer* QRhiVulkanExHelper::newVkBuffer(QRhi* inRhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size) {
	QRhiImplementation* ptr = (*(QRhiImplementation**)inRhi);
	return new QVkBufferEx(ptr, type, flags, size);
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
	devInfo.enabledLayerCount = uint32_t(devLayers.size());
	devInfo.ppEnabledLayerNames = devLayers.constData();
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
		if (caps.apiVersion >= QVersionNumber(1, 3))
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
	handles.vmemAllocator = vmaallocator;
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

