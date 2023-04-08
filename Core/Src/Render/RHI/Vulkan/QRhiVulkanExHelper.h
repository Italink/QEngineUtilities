#ifndef QRhiVulkanExHelper_h__
#define QRhiVulkanExHelper_h__

#include "private/qrhivulkan_p.h"
#include "private/qrhi_p.h"
#include "qsharedpointer.h"

namespace QRhiVulkanExHelper {
	QRhiBuffer* newVkBuffer(QRhi* inRhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size);

	QRhiVulkanNativeHandles createVulkanNativeHandles(const QRhiVulkanInitParams& params);

	void destroyVulkanNativeHandles(const QRhiVulkanNativeHandles& handles);
};


#endif // QRhiVulkanExHelper_h__
