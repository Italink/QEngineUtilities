#include "Render/RHI/QRhiHelper.h"
#include <QFile>


#include "private/qrhivulkan_p.h"

#ifndef QT_NO_OPENGL
#include <QOffscreenSurface>
#include "private/qrhigles2_p.h"
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
#include "rhi/qshaderbaker.h"
#else
#include "private/qshaderbaker_p.h"
#include "private/qrhi_p_p.h"
#include "private/qrhivulkanext_p.h"
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include <QtGui/private/qrhimetal_p.h>
#endif

#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>

#include <qmath.h>
#include <QVulkanFunctions>

#include "Vulkan/QRhiVulkanExHelper.h"

QShaderDefinitions& QShaderDefinitions::addDefinition(const QByteArray def) {
	mDefinitions.append("#define " + def + "\n");
	return *this;
}

QShaderDefinitions& QShaderDefinitions::addDefinition(const QByteArray def, int value) {
	mDefinitions.append("#define " + def +" " + QString::number(value).toLocal8Bit() + "\n");
	return *this;
}

void QRhiSignal::request() {
	bDirty = true;
}

bool QRhiSignal::ensure()
{
	bool var = bDirty;
	bDirty = false;
	return var;
}

bool QRhiSignal::peek() {
	return bDirty;
}

QSharedPointer<QRhi> QRhiHelper::create(QRhi::Implementation inBackend, QRhi::Flags inFlags, QWindow* inWindow)
{
	QSharedPointer<QRhi> rhi;
	if (inBackend == QRhi::Null) {
		QRhiNullInitParams params;
		rhi.reset(static_cast<QRhi*>(QRhi::create(QRhi::Null, &params, inFlags)));
	}

#ifndef QT_NO_OPENGL
	if (inBackend == QRhi::OpenGLES2) {
		QRhiGles2InitParams params;
		params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
		params.window = inWindow;
		rhi.reset(static_cast<QRhi*>(QRhi::create(QRhi::OpenGLES2, &params, inFlags)));
	}
#endif

#if QT_CONFIG(vulkan)
	if (inBackend == QRhi::Vulkan) {
		QVulkanInstance* vkInstance = QRhiVulkanExHelper::instance();
		QRhiVulkanInitParams params;
		if (inWindow) {
			inWindow->setVulkanInstance(vkInstance);
			params.window = inWindow;
		}
		params.inst = vkInstance;
		auto importedHandles = QRhiVulkanExHelper::createVulkanNativeHandles(params);
		rhi.reset(static_cast<QRhi*>(QRhi::create(QRhi::Vulkan, &params, inFlags, &importedHandles)), [importedHandles](QRhi* rhi) {
			delete rhi;
			QRhiVulkanExHelper::destroyVulkanNativeHandles(importedHandles);
		});
	}
#endif

#ifdef Q_OS_WIN
	if (inBackend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		params.enableDebugLayer = true;
		rhi.reset(static_cast<QRhi*>(QRhi::create(QRhi::D3D11, &params, inFlags)));
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	if (inBackend == QRhi::D3D12) {
		QRhiD3D12InitParams params;
		params.enableDebugLayer = true;
		rhi.reset(static_cast<QRhi*>(QRhi::create(QRhi::D3D12, &params, inFlags)));
	}
#endif

#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
	if (inBackend == QRhi::Metal) {
		QRhiMetalInitParams params;
		rhi.reset(static_cast<QRhi*>(QRhi::create(QRhi::Metal, &params, inFlags)));
	}
#endif
	return rhi;
}

QShader QRhiHelper::newShaderFromCode(QRhi* rhi, QShader::Stage stage, QByteArray code, QByteArray preamble)
{
	QShaderBaker baker;
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	QList<QShaderBaker::GeneratedShader> generatedShaders;
	generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::SpirvShader,QShaderVersion(100) };
	if (rhi->backend() == QRhi::Vulkan || rhi->backend()  == QRhi::OpenGLES2) {
		generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::GlslShader,QShaderVersion(450) };
	}
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
	else if (rhi->backend()  == QRhi::D3D11 || rhi->backend()  == QRhi::D3D12) {
		generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::HlslShader,QShaderVersion(50) };
		code = QString(code).replace("imageCube", "image2DArray").toLocal8Bit();
	}
#endif
	else if (rhi->backend()  == QRhi::Metal) {
		generatedShaders << QShaderBaker::GeneratedShader{ QShader::Source::MslShader,QShaderVersion(20) };
	}
	baker.setGeneratedShaders(generatedShaders);
	baker.setSourceString(code, stage);

	baker.setPreamble(preamble);
	QShader shader = baker.bake();
	if (!shader.isValid()) {
		QStringList codelist = QString(code).split('\n');
		for (int i = 0; i < codelist.size(); i++) {
			qWarning() << i + 1 << codelist[i].toLocal8Bit().data();
		}
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader QRhiHelper::newShaderFromQSBFile(const char* filename)
{
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

QRhiBuffer* QRhiHelper::newVkBuffer(QRhi* rhi, QRhiBuffer::Type type, VkBufferUsageFlags flags, int size)
{
	return QRhiVulkanExHelper::newVkBuffer(rhi, type, flags, size);
}

void QRhiHelper::setShaderResources(QRhiResource* pipeline, QRhiCommandBuffer* cb, QRhiShaderResourceBindings* srb, int dynamicOffsetCount, const QRhiCommandBuffer::DynamicOffset* dynamicOffsets)
{
	QRhiVulkanExHelper::setShaderResources(pipeline, cb, srb, dynamicOffsetCount, dynamicOffsets);
}