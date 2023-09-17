#ifndef QRhiVulkanExHelper_h__
#define QRhiVulkanExHelper_h__

#include "private/qrhivulkan_p.h"
#include "private/qrhi_p.h"
#include "qsharedpointer.h"
#include "QEngineCoreAPI.h"


namespace QRhiVulkanExHelper {
	QENGINECORE_API QVulkanInstance* instance();

	QENGINECORE_API QRhiBuffer* newVkBuffer(QRhi* inRhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size);

	QENGINECORE_API void updateShaderResourceBindings(QVulkanDeviceFunctions* df, VkDevice dev, QRhiVulkan* rhi, QRhiShaderResourceBindings* srb, int descSetIdx = -1);

	QENGINECORE_API VkImageView imageViewForLevel(QVkTexture* texture, int level);

	QENGINECORE_API void setShaderResources(QRhiResource* pipeline, QRhiCommandBuffer* cb, QRhiShaderResourceBindings* srb = nullptr, int dynamicOffsetCount = 0, const QRhiCommandBuffer::DynamicOffset* dynamicOffsets = nullptr);

	QENGINECORE_API QRhiVulkanNativeHandles createVulkanNativeHandles(const QRhiVulkanInitParams& params);

	QENGINECORE_API void destroyVulkanNativeHandles(const QRhiVulkanNativeHandles& handles);
};


#endif // QRhiVulkanExHelper_h__
