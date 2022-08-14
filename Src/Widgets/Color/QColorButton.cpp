#include "QColorButton.hpp"
#include "QtColorDialog.h"
#include <QPainter>
#include "Core\QDetailWidgetStyleManager.h"

QColorButton::QColorButton(QColor color)
	: mColor(color)
{
	setMinimumWidth(100);
	setFixedHeight(20);
	SetColor(color);
}

void QColorButton::SetColor(QColor color)
{
	mColor = color;
	update();
}

QColor QColorButton::GetColor() const
{
	return mColor;
}

void QColorButton::paintEvent(QPaintEvent* event)
{
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);
	painter.setBrush(mColor);
	painter.drawRoundedRect(rect().adjusted(2,2,-2,-2), 2, 2);
}

void QColorButton::mousePressEvent(QMouseEvent* event) {
	QHoverWidget::mousePressEvent(event);
	QRect geom = rect();
	geom.moveTopLeft(mapToGlobal(QPoint(0, 0)));
	QtColorDialog::CreateAndShow(mColor, geom);
	QtColorDialog::Current->setStyleSheet(QDetailWidgetStyleManager::Instance()->GetStylesheet());
	QObject::connect(QtColorDialog::Current, &QtColorDialog::AsColorChanged, this, [&](const QColor& color) {
		SetColor(color);
		Q_EMIT AsColorChanged(mColor);
	});
}
