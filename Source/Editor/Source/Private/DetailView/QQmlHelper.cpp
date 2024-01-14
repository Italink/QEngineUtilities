#include "QQmlHelper.h"
#include <QCursor>

QQmlHelper* QQmlHelper::Get()
{
	static QQmlHelper Ins;
	return &Ins;
}

QString QQmlHelper::numberToString(QVariant var, int precision)
{
	double value = var.toDouble();
	return QString::number(value, 'f', precision);
}

void QQmlHelper::setCursorPos(qreal x, qreal y)
{
	//qDebug() << QCursor::pos();
	//QCursor::setPos(x, y);
}

void QQmlHelper::setCursorPosTest(QQuickItem* item, qreal x, qreal y)
{
	QPointF global = item->mapToGlobal(x, y);
	QCursor::setPos(global.x(), global.y());
	qDebug() << x << y << global << QCursor::pos();
}
