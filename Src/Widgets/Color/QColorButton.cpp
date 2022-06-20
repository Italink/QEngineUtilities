#include "QColorButton.hpp"
#include "QtColorDialog.h"
#include <QPainter>

QColorButton::QColorButton(QColor color)
	: mColor(color)
{
	setMinimumWidth(100);
	setFixedHeight(20);
	SetColor(color);
	QObject::connect(this, &QPushButton::clicked, this, [&]() {
		QtColorDialog* dialog = new QtColorDialog;
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->SetColor(mColor);
		QObject::connect(dialog, &QtColorDialog::AsColorChanged, this, [&](const QColor& color) {
			SetColor(color);
			Q_EMIT AsColorChanged(mColor);
		});
		dialog->show();
	});
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
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);
	painter.setBrush(mColor);
	painter.drawRoundedRect(rect(), 2, 2);
}