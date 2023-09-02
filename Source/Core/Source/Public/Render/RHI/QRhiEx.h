#ifndef QRhiEx_h__
#define QRhiEx_h__

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

class QENGINECORE_API QRhiEx: public QRhi {
public:
	static QSharedPointer<QRhiEx> newRhiEx(QRhi::Implementation inBackend = QRhi::Vulkan, QRhi::Flags inFlags = QRhi::Flag(), QWindow* inWindow = nullptr);

	QShader newShaderFromCode(QShader::Stage stage, QByteArray code, QByteArray preamble = QByteArray());

	QRhiBuffer* newVkBuffer(QRhiBuffer::Type type, VkBufferUsageFlags flags, int size);

	static QShader newShaderFromQSBFile(const char* filename);
public:
	class QENGINECORE_API Signal {
	public:
		Signal() {};
		void request();
		bool ensure();
		bool peek();
	private:
		bool bDirty = false;
	};
};

#endif // QRhiEx_h__
