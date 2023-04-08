#include "QRhiEx.h"
#include <QFile>
#include "private/qshaderbaker_p.h"
#include "private/qrhivulkan_p.h"
#include "private/qrhi_p_p.h"
#ifndef QT_NO_OPENGL
#include <QOffscreenSurface>
#include "private/qrhigles2_p.h"
#endif

#ifdef Q_OS_WIN
#include <QtGui/private/qrhid3d11_p.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include <QtGui/private/qrhid3d12_p.h>
#endif
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include <QtGui/private/qrhimetal_p.h>
#endif

#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>

#include "private/qrhivulkanext_p.h"
#include "private/qvulkandefaultinstance_p.h"

#include <qmath.h>
#include <QVulkanFunctions>

#include "Vulkan/QRhiVulkanExHelper.h"

void QRhiEx::Signal::request() {
	bDirty = true;
}


bool QRhiEx::Signal::receive()
{
	bool var = bDirty;
	bDirty = false;
	return var;
}

bool QRhiEx::Signal::peek() {
	return bDirty;
}

QRhiEx* QRhiEx::newRhiEx(QRhi::Implementation inBackend /*= QRhi::Vulkan*/, QRhi::Flags inFlags /*= QRhi::Flag()*/, QWindow* inWindow /*= nullptr*/) {
	if (inBackend == QRhi::Null) {
		QRhiNullInitParams params;
		return static_cast<QRhiEx*>(QRhi::create(QRhi::Null, &params, inFlags));
	}

#ifndef QT_NO_OPENGL
	if (inBackend == QRhi::OpenGLES2) {
		QRhiGles2InitParams params;
		params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
		params.window = inWindow;
		return static_cast<QRhiEx*>(QRhi::create(QRhi::OpenGLES2, &params, inFlags));
	}
#endif

#if QT_CONFIG(vulkan)
	if (inBackend == QRhi::Vulkan) {
		QVulkanDefaultInstance::setFlag(QVulkanDefaultInstance::EnableValidation);
		QVulkanInstance* vkInstance = QVulkanDefaultInstance::instance();
		QRhiVulkanInitParams params;
		if (inWindow) {
			inWindow->setVulkanInstance(vkInstance);
			params.window = inWindow;
		}
		params.inst = vkInstance;
		auto importedHandles = QRhiVulkanExHelper::createVulkanNativeHandles(params);
		return static_cast<QRhiEx*>(QRhi::create(QRhi::Vulkan, &params, inFlags, &importedHandles));
	}
#endif

#ifdef Q_OS_WIN
	if (inBackend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		params.enableDebugLayer = true;
		return static_cast<QRhiEx*>(QRhi::create(QRhi::D3D11, &params, inFlags));
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	if (inBackend == QRhi::D3D12) {
		QRhiD3D12InitParams params;
		params.enableDebugLayer = true;
		return static_cast<QRhiEx*>(QRhi::create(QRhi::D3D12, &params, inFlags));
	}
#endif

#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
	if (inBackend == QRhi::Metal) {
		QRhiMetalInitParams params;
		return static_cast<QRhiEx*>(QRhi::create(QRhi::Metal, &params, inFlags));
	}
#endif
	return nullptr;
}

QShader QRhiEx::newShaderFromCode(QShader::Stage stage, QByteArray code) {
	QShaderBaker baker;
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	QList<QShaderBaker::GeneratedShader> generatedShaders;
	generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::SpirvShader,QShaderVersion(100) };
	if (backend() == QRhi::Vulkan || backend() == QRhi::OpenGLES2) {
		generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::GlslShader,QShaderVersion(450) };
	}
	else if (backend() == QRhi::D3D11|| backend() == QRhi::D3D12) {
		generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::HlslShader,QShaderVersion(50) };
		code = QString(code).replace("imageCube", "image2DArray").toLocal8Bit();
	}
	else if (backend() == QRhi::Metal) {
		generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::MslShader,QShaderVersion(20) };
	}
	baker.setGeneratedShaders(generatedShaders);
	baker.setSourceString(code, stage);
	QShader shader = baker.bake();
	if (!shader.isValid()) {
		QStringList codelist = QString(code).split('\n');
		for (int i = 0; i < codelist.size(); i++) {
			qWarning() << i+1 << codelist[i].toLocal8Bit().data();
		}
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader QRhiEx::newShaderFromQSBFile(const char* filename) {
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

QRhiBuffer* QRhiEx::newVkBuffer(QRhiBuffer::Type type, VkBufferUsageFlags flags, int size) {
	return QRhiVulkanExHelper::newVkBuffer(this,type,flags,size);
}