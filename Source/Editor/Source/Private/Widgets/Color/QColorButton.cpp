#include "Widgets/Color/QColorButton.h"
#include <QPainter>
#include "Widgets/Color/QtColorDialog.h"
#include "qevent.h"
#include "QEngineEditorStyleManager.h"

QColorButton::QColorButton(QColor color)
	: mColor(color)
{
	setMinimumWidth(100);
	setFixedHeight(20);
	setColor(color);
}

void QColorButton::setColor(QColor color)
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
	geom.moveTopLeft(event->globalPos());
	QtColorDialog::CreateAndShow(mColor, geom);
	QObject::connect(QtColorDialog::Current, &QtColorDialog::asColorChanged, this, [&](const QColor& color) {
		setColor(color);
		Q_EMIT asColorChanged(mColor);
	});
}
