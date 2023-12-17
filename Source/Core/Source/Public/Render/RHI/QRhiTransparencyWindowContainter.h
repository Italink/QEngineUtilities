#ifndef QRhiTransparencyWindowContainter_h__
#define QRhiTransparencyWindowContainter_h__

#include <QWidget>
#include "private/qwidget_p.h"
#include "qtclasshelpermacros.h"
#include "QEngineCoreAPI.h"

class QRhiTransparencyWindowContainterPrivate;

class QENGINECORE_API QRhiTransparencyWindowContainter: public QWidget
{
	Q_DECLARE_PRIVATE(QRhiTransparencyWindowContainter);
public:
	static QWidget* create(QWindow* window, QRhi::Implementation api);
private:
	QRhiTransparencyWindowContainter(QRhi::Implementation api);
private:
	QPlatformBackingStoreRhiConfig mConfig;
};

class QRhiTransparencyWindowContainterPrivate : public QWidgetPrivate
{
	Q_DECLARE_PUBLIC(QRhiTransparencyWindowContainter);
public:
	QPlatformBackingStoreRhiConfig rhiConfig() const override;
};


#endif // QRhiTransparencyWindowContainter_h__
