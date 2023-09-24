#ifndef QRhiVulkanExHelper_h__
#define QRhiVulkanExHelper_h__

#include "private/qrhivulkan_p.h"
#include "private/qrhi_p.h"
#include "qsharedpointer.h"

namespace QRhiVulkanExHelper {
	QVulkanInstance* instance();

	QRhiBuffer* newVkBuffer(QRhi* inRhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size);

	void updateShaderResourceBindings(QVulkanDeviceFunctions* df, VkDevice dev, QRhiVulkan* rhi, QRhiShaderResourceBindings* srb, int descSetIdx = -1);

	VkImageView imageViewForLevel(QVkTexture* texture, int level);

	void setShaderResources(QRhiResource* pipeline, QRhiCommandBuffer* cb, QRhiShaderResourceBindings* srb = nullptr, int dynamicOffsetCount = 0, const QRhiCommandBuffer::DynamicOffset* dynamicOffsets = nullptr);

	QRhiVulkanNativeHandles createVulkanNativeHandles(const QRhiVulkanInitParams& params);

	void destroyVulkanNativeHandles(const QRhiVulkanNativeHandles& handles);
};


#endif // QRhiVulkanExHelper_h__
