#include "QColors.h"

QColors::QColors()
{
	mStops.push_back({ 0.5,Qt::white });
}

QColors::QColors(QGradientStops stops)
	:mStops(stops)
{
}

QGradientStops QColors::getStops() const
{
	return mStops;
}

void QColors::setStops(QGradientStops val)
{
	mStops = val;
}

QDataStream& operator>>(QDataStream& in, QColors& var)
{
	in >> var.mStops;
	return in;
}

QDataStream& operator<<(QDataStream& out, const QColors& var)
{
	out << var.mStops;
	return out;
}

bool QColors::operator==(const QColors& other)
{
	return other.mStops == this->mStops;
}