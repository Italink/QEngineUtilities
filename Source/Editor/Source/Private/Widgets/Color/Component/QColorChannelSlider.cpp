#include "QColorChannelSlider.hpp"
#include "QBoxLayout"
#include "QPainter"

QColorChannelSlider::QColorChannelSlider(QString inName, float inDefault, float inMin, float inMax)
	: mLbName(inName)
	, mValueBox(inDefault,true,inMin,inMax)
{
	QHBoxLayout* h = new QHBoxLayout(this);
	QVBoxLayout* v = new QVBoxLayout;

	h->setContentsMargins(0, 0, 0, 0);
	v->setContentsMargins(0, 0, 0, 0);
	v->setSpacing(0);

	h->addWidget(&mLbName);
	h->addLayout(v);

	v->addWidget(&mValueBox);
	v->addSpacing(5);

	connect(&mValueBox, &QNumberBox::asValueChanged, this, [this](QVariant var) {
		Q_EMIT asValueChanged(var.toFloat());
	});
}

void QColorChannelSlider::SetGradientStops(const QGradientStops& inStops)
{
	mGradientStops = inStops;
	update();
}

void QColorChannelSlider::SetChannelValue(float inValue)
{
	mValueBox.setVar(inValue);
}

void QColorChannelSlider::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect colorRect(0, 0, mValueBox.width(), 5);
	colorRect.moveTopLeft(mValueBox.geometry().bottomLeft());
	QLinearGradient linear;
	linear.setStart(colorRect.bottomLeft());
	linear.setFinalStop(colorRect.bottomRight());
	linear.setStops(mGradientStops);
	painter.fillRect(colorRect, linear);
}

