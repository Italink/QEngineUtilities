#include "GradientBarItem.hpp"
#include "QPainter"
#include "qevent.h"

GradientBarItem::GradientBarItem(QGradientStop stop, QWidget* parent /*= 0*/)
	: QWidget(parent)
	, gradientStop(stop)
{
	setFixedSize(16, 20);
	setCursor(Qt::PointingHandCursor);
}

GradientBarItem::~GradientBarItem()
{
}

QColor GradientBarItem::getColor() const
{
	return gradientStop.second;
}

void GradientBarItem::setColor(QColor val)
{
	gradientStop.second = val;
	update();
}

qreal GradientBarItem::getPos() const
{
	return gradientStop.first;
}

bool GradientBarItem::setPos(qreal val)
{
	val = qBound(0.0, val, 1.0);
	if (val == gradientStop.first)
		return false;
	gradientStop.first = val;
	return true;
}

void GradientBarItem::paintEvent(QPaintEvent* ev)
{
	QPainter painter(this);
	painter.setPen(QColor(150, 150, 150));
	QPolygon p;
	painter.setBrush(getColor());
	p.setPoints(5, 8, 2, 14, 8, 14, 18, 2, 18, 2, 8);
	painter.drawPolygon(p);
	if (isHighlight()) {
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QColor(0, 176, 240));
		p.setPoints(5, 8, 1, 15, 8, 15, 19, 1, 19, 1, 8);
		painter.drawPolygon(p);
	}
}

void GradientBarItem::mousePressEvent(QMouseEvent* ev)
{
	if (ev->buttons() & Qt::LeftButton) {
		if (!isHighlight()) {
			emit itemHighLight(this);
		}
	}
}

void GradientBarItem::mouseMoveEvent(QMouseEvent* ev)
{
	if (ev->buttons() & Qt::LeftButton && isHighlight()) {
		if (setPos((mapToParent(ev->pos()).x()) / (double)(parentWidget()->width())))
			emit itemMoved(this);
	}
}

void GradientBarItem::mouseReleaseEvent(QMouseEvent* event) {
	if (isHighlight() && event->pos().y() > 50) {
		emit itemRemoved(this);
	}
}