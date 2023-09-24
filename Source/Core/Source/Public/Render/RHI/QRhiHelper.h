#ifndef QRhi_h__
#define QRhi_h__

#include "private/qrhi_p.h"
#include "private/qrhivulkan_p.h"
#include <QWindow>
#include "QEngineCoreAPI.h"

class QENGINECORE_API QShaderDefinitions {
public:
	QShaderDefinitions& addDefinition(const QByteArray def);
	QShaderDefinitions& addDefinition(const QByteArray def, int value);
	operator QByteArray() { return mDefinitions; }
private:
	QByteArray mDefinitions;
};

class QENGINECORE_API QRhiSignal {
public:
	QRhiSignal() {};
	void request();
	bool ensure();
	bool peek();
private:
	bool bDirty = false;
};

struct QENGINECORE_API QRhiHelper {
	struct QENGINECORE_API InitParams {
		QRhi::Implementation backend = QRhi::Vulkan;
		QRhi::Flags rhiFlags = QRhi::Flag();
		QRhiSwapChain::Flags swapChainFlags = QRhiSwapChain::Flag::NoVSync;
		QRhi::BeginFrameFlags beginFrameFlags;
		QRhi::EndFrameFlags endFrameFlags;
		int sampleCount = 1;
		bool enableStat = false;
	};

	struct QENGINECORE_API InitParamsBuilder {
		InitParamsBuilder& backend(QRhi::Implementation var) { params.backend = var; return *this; }
		InitParamsBuilder& rhiFlags(QRhi::Flags var) { params.rhiFlags = var; return *this; }
		InitParamsBuilder& swapChainFlags(QRhiSwapChain::Flags var) { params.swapChainFlags = var; return *this; }
		InitParamsBuilder& beginFrameFlags(QRhi::BeginFrameFlags var) { params.beginFrameFlags = var; return *this; }
		InitParamsBuilder& endFrameFlags(QRhi::EndFrameFlags var) { params.endFrameFlags = var; return *this; }
		InitParamsBuilder& sampleCount(int var) { params.sampleCount = var; return *this; }
		InitParamsBuilder& enableStat(bool var) { params.enableStat = var; return *this; }

		operator InitParams() { return params; }
	private:
		InitParams params;
	};
	static QSharedPointer<QRhi> create(QRhi::Implementation inBackend = QRhi::Vulkan, QRhi::Flags inFlags = QRhi::Flag(), QWindow* inWindow = nullptr);

	static QShader newShaderFromCode(QShader::Stage stage, QByteArray code, QByteArray preamble = QByteArray());

	static QShader newShaderFromQSBFile(const char* filename);

	static QRhiBuffer* newVkBuffer(QRhi* rhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size);

	static void setShaderResources(QRhiResource* pipeline, QRhiCommandBuffer* cb, QRhiShaderResourceBindings* srb = nullptr, int dynamicOffsetCount = 0, const QRhiCommandBuffer::DynamicOffset* dynamicOffsets = nullptr);
};

#endif // QRhi_h__
