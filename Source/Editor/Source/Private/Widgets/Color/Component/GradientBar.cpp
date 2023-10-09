/**
 * \file gradient_slider.cpp
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2020 Mattia Basaglia
 * \copyright Copyright (C) 2014 Calle Laakkonen
 * \copyright Copyright (C) 2017 caryoscelus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QPainter>
#include <QStyleOptionSlider>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QDebug>
#include "GradientBar.hpp"
#include "GradientBarItem.hpp"

class GradientBar::Private
{
public:
	QLinearGradient gradient;
	QList<GradientBarItem*> items;
	GradientBarItem* currentItem;
	QBrush back;
	Private()
		: back(Qt::darkGray, Qt::DiagCrossPattern)
		, currentItem(nullptr)
	{
		//loadResource();
		back.setTexture(QPixmap(QStringLiteral(":/Resources/Icons/alphaback.png")));
		gradient.setCoordinateMode(QLinearGradient::StretchToDeviceMode);
		gradient.setSpread(QLinearGradient::RepeatSpread);
	}
};

GradientBar::GradientBar(QWidget* parent) :
	QWidget(parent), p(new Private)
{
	setMinimumSize(120, 30);
	flushGradinetBar();
}

GradientBar::~GradientBar()
{
	delete p;
}

QGradientStops GradientBar::colors() const
{
	return p->gradient.stops();
}

void GradientBar::setColors(const QGradientStops& colors)
{
	if (colors.isEmpty())
		return;
	lockSignal = true;
	clear();
	for (auto& color : colors) {
		addItem(color);
	}
	flushGradinetBar();
	lockSignal = false;
	emit currentItemChanged(getCurrentColor());
	update();
}

void GradientBar::clear()
{
	while (p->items.count()) {
		p->items.takeLast()->deleteLater();
	}
}

QColor GradientBar::getCurrentColor() const
{
	if (p->currentItem == nullptr)
		return QColor();
	return p->currentItem->getColor();
}

void GradientBar::setCurrentColor(QColor color)
{
	if (p->currentItem != nullptr)
		p->currentItem->setColor(color);
	flushGradinetBar();
}

GradientBarItem* GradientBar::addItem(QGradientStop stop)
{
	GradientBarItem* item = new GradientBarItem(stop, this);
	p->items.push_back(item);
	updateItemPositon(item);
	setCurrentItem(item);
	connect(item, &GradientBarItem::itemMoved, this, [this](GradientBarItem* item) {
		updateItemPositon(item);
		flushGradinetBar();
		emit currentItemPositionChanged(item->getPos());
	});
	connect(item, &GradientBarItem::itemRemoved, this, &GradientBar::removeItem);
	connect(item, &GradientBarItem::itemHighLight, this, &GradientBar::setCurrentItem);
	if (this->isVisible())
		item->show();
	return item;
}

void GradientBar::removeItem(GradientBarItem* item) {
	if (p->items.size() <= 1) {
		return;
	}
	p->items.removeOne(item);
	if (item == p->currentItem) {
		setCurrentItem(p->items.front());
	}
	item->deleteLater();
}

void GradientBar::updateItemPositon(GradientBarItem* item)
{
	item->move(this->width() * item->getPos() - item->width() / 2, height() - 20);
}

void GradientBar::setCurrentItem(GradientBarItem* item)
{
	if (p->currentItem == item)
		return;
	if (p->currentItem != nullptr) {
		p->currentItem->setHighlight(false);
	}
	p->currentItem = item;
	p->currentItem->setHighlight(true);
	if (!lockSignal)
		emit currentItemChanged(item->getColor());
}

void GradientBar::flushGradinetBar() {
	QGradientStops stops;
	for (auto& it : p->items)
		stops.push_back({ it->getPos(),it->getColor() });
	p->gradient.setStops(stops);
	update();
}

void GradientBar::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	p->gradient.setFinalStop(1, 0);
	painter.setPen(Qt::NoPen);
	painter.setBrush(p->back);
	painter.drawRect(1, 1, geometry().width() - 2, geometry().height() - 12);
	painter.setBrush(p->gradient);
	painter.drawRect(1, 1, geometry().width() - 2, geometry().height() - 12);
}

void GradientBar::mouseReleaseEvent(QMouseEvent* ev)
{
	double fatcor = ev->position().x() / width();
	addItem({ fatcor,gradientBlendedColor(p->gradient.stops(),fatcor) });
}

void GradientBar::resizeEvent(QResizeEvent* event)
{
	for (auto item : p->items)
		updateItemPositon(item);
}