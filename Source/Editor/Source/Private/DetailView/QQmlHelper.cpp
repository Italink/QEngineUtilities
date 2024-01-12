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
	 QCursor::setPos(x, y);
}
