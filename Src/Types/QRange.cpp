#include "QRange.h"

QRange::QRange(int lower, int upper, int min, int max)
	: lower_(lower)
	, upper_(upper)
	, mMin(min)
	, mMax(max)
{
}

void QRange::setLimite(int min, int max)
{
	mMin = min;
	mMax = max;
}

int QRange::min() const
{
	return mMin;
}

void QRange::setMin(int min)
{
	mMin = min;
}

int QRange::max() const
{
	return mMax;
}

void QRange::setMax(int max)
{
	mMax = max;
}

int QRange::getLower() const
{
	return lower_;
}

void QRange::setLower(int val){
	lower_ = std::clamp(val,mMin,mMax);
}

int QRange::getUpper() const
{
	return upper_;
}

void QRange::setUpper(int val){
	upper_ = std::clamp(val, mMin, mMax);
}

void QRange::setRange(int lower, int upper)
{
	lower_ = lower;
	upper_ = upper;
}

QDebug operator<<(QDebug debug, const QRange& range)
{
	debug <<"QRange(" << range.getLower()<< ", " << range.getUpper()<<")";
	return debug;
}

bool QRange::operator==(const QRange& other)
{
	return this->min() == other.min() &&
		this->max() == other.max() &&
		this->getLower() == other.getLower() &&
		this->getUpper() == other.getUpper();
}

QDataStream& operator<<(QDataStream& out, const QRange& var)
{
	out << var.lower_ << var.upper_ << var.min() << var.max();
	return out;
}

QDataStream& operator>>(QDataStream& in, QRange& var)
{
	in >> var.lower_ >> var.upper_ >> var.mMin >> var.mMax;
	return in;
}