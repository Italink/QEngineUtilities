#include "QRhiTransparencyWindowContainter.h"
#include <QHBoxLayout>

QWidget* QRhiTransparencyWindowContainter::create(QWindow* window, QRhi::Implementation api)
{
	QRhiTransparencyWindowContainter* containter = new QRhiTransparencyWindowContainter(api);
	QHBoxLayout* h = new QHBoxLayout(containter);
	h->setContentsMargins(0, 0, 0, 0);
	h->addWidget(QWidget::createWindowContainer(window));
	return containter;
}

QRhiTransparencyWindowContainter::QRhiTransparencyWindowContainter(QRhi::Implementation api)
	: QWidget(*(new QRhiTransparencyWindowContainterPrivate), nullptr, Qt::WindowFlags())
{
	mConfig.setEnabled(true);
	switch (api)
	{
	case QRhi::Null:
		mConfig.setApi(QPlatformBackingStoreRhiConfig::Null);
		break;
	case QRhi::Vulkan:
		mConfig.setApi(QPlatformBackingStoreRhiConfig::Vulkan);
		break;
	case QRhi::OpenGLES2:
		mConfig.setApi(QPlatformBackingStoreRhiConfig::OpenGL);
		break;
	case QRhi::D3D11:
		mConfig.setApi(QPlatformBackingStoreRhiConfig::D3D11);
		break;
	case QRhi::Metal:
		mConfig.setApi(QPlatformBackingStoreRhiConfig::Metal);
		break;
	case QRhi::D3D12:
		mConfig.setApi(QPlatformBackingStoreRhiConfig::D3D12);
		break;
	default:
		break;
	}
}

QPlatformBackingStoreRhiConfig QRhiTransparencyWindowContainterPrivate::rhiConfig() const
{
	return q_func()->mConfig;
}
