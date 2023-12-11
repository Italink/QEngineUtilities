#include "Widgets/QVectorBox.h"
#include "QBoxLayout"
#include "QPainter"

QVector4DBox::QVector4DBox(QVector4D vec)
	: mX(0.0f)
	, mY(0.0f)
	, mZ(0.0f)
	, mW(0.0f)
{
	setValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX,0, Qt::AlignLeft);
	h->addWidget(&mY,0, Qt::AlignLeft);
	h->addWidget(&mZ,0, Qt::AlignLeft);
	h->addWidget(&mW,0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::asValueChanged, this, &QVector4DBox::emitValueChanged);
	connect(&mY, &QNumberBox::asValueChanged, this, &QVector4DBox::emitValueChanged);
	connect(&mZ, &QNumberBox::asValueChanged, this, &QVector4DBox::emitValueChanged);
	connect(&mW, &QNumberBox::asValueChanged, this, &QVector4DBox::emitValueChanged);
}

void QVector4DBox::setValue(QVector4D vec)
{
	blockSignals(true);
	mX.setVar(vec.x());
	mY.setVar(vec.y());
	mZ.setVar(vec.z());
	mW.setVar(vec.w());
	blockSignals(false);
	emitValueChanged(QVariant());
}

QVector4D QVector4DBox::getValue()
{
	return QVector4D(
		mX.getVar().toFloat(),
		mY.getVar().toFloat(),
		mZ.getVar().toFloat(),
		mW.getVar().toFloat()
	);
}

void QVector4DBox::emitValueChanged(QVariant)
{
	Q_EMIT asValueChanged(getValue());
}

void QVector4DBox::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect drawRect(0, (height() - mX.height()) / 2, 4, mX.height());
	drawRect.moveRight(mX.geometry().left());
	painter.fillRect(drawRect, QColor(221, 81, 69));
	drawRect.moveRight(mY.geometry().left());
	painter.fillRect(drawRect, QColor(25, 161, 95));
	drawRect.moveRight(mZ.geometry().left());
	painter.fillRect(drawRect, QColor(76, 139, 244));
	drawRect.moveRight(mW.geometry().left());
	painter.fillRect(drawRect, QColor(255, 206, 68));
}

QVector3DBox::QVector3DBox(QVector3D vec)
	: mX(0.0f)
	, mY(0.0f)
	, mZ(0.0f)
{
	setValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX,0, Qt::AlignLeft);
	h->addWidget(&mY,0, Qt::AlignLeft);
	h->addWidget(&mZ,0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::asValueChanged, this, &QVector3DBox::emitValueChanged);
	connect(&mY, &QNumberBox::asValueChanged, this, &QVector3DBox::emitValueChanged);
	connect(&mZ, &QNumberBox::asValueChanged, this, &QVector3DBox::emitValueChanged);
}

void QVector3DBox::setValue(QVector3D vec)
{
	blockSignals(true);
	mX.setVar(vec.x());
	mY.setVar(vec.y());
	mZ.setVar(vec.z());
	blockSignals(false);
	emitValueChanged(QVariant());
}

QVector3D QVector3DBox::getValue()
{
	return QVector3D(
		mX.getVar().toFloat(),
		mY.getVar().toFloat(),
		mZ.getVar().toFloat()
	);
}

void QVector3DBox::emitValueChanged(QVariant)
{
	Q_EMIT asValueChanged(getValue());
}

void QVector3DBox::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect drawRect(0, (height() - mX.height()) / 2, 4, mX.height());
	drawRect.moveRight(mX.geometry().left());
	painter.fillRect(drawRect, QColor(221, 81, 69));
	drawRect.moveRight(mY.geometry().left());
	painter.fillRect(drawRect, QColor(25, 161, 95));
	drawRect.moveRight(mZ.geometry().left());
	painter.fillRect(drawRect, QColor(76, 139, 244));
}

QVector2DBox::QVector2DBox(QVector2D vec)
	: mX(0.0f)
	, mY(0.0f)
{
	setValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX, 0, Qt::AlignLeft);
	h->addWidget(&mY ,0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::asValueChanged, this,&QVector2DBox::emitValueChanged);
	connect(&mY, &QNumberBox::asValueChanged, this,&QVector2DBox::emitValueChanged);
}

void QVector2DBox::setValue(QVector2D vec)
{
	blockSignals(true);
	mX.setVar(vec.x());
	mY.setVar(vec.y());
	blockSignals(false);
	emitValueChanged(QVariant());
}

QVector2D QVector2DBox::getValue()
{
	return QVector2D(
		mX.getVar().toFloat(),
		mY.getVar().toFloat()
	);
}

void QVector2DBox::emitValueChanged(QVariant)
{
	Q_EMIT asValueChanged(getValue());
}

void QVector2DBox::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect drawRect(0, (height() - mX.height()) / 2, 4, mX.height());
	drawRect.moveRight(mX.geometry().left());
	painter.fillRect(drawRect, QColor(221, 81, 69));
	drawRect.moveRight(mY.geometry().left());
	painter.fillRect(drawRect, QColor(25, 161, 95));
}

QPointBox::QPointBox(QPoint vec)
	: mX(0)
	, mY(0)
{
	setValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX, 0, Qt::AlignLeft);
	h->addWidget(&mY, 0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::asValueChanged, this, &QPointBox::emitValueChanged);
	connect(&mY, &QNumberBox::asValueChanged, this, &QPointBox::emitValueChanged);
}

void QPointBox::setValue(QPoint vec)
{
	blockSignals(true);
	mX.setVar(vec.x());
	mY.setVar(vec.y());
	blockSignals(false);
	emitValueChanged(QVariant());
}

QPoint QPointBox::getValue()
{
	return QPoint(
		mX.getVar().toInt(),
		mY.getVar().toInt()
	);
}

void QPointBox::emitValueChanged(QVariant)
{
	Q_EMIT asValueChanged(getValue());
}

void QPointBox::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect drawRect(0, (height() - mX.height()) / 2, 4, mX.height());
	drawRect.moveRight(mX.geometry().left());
	painter.fillRect(drawRect, QColor(221, 81, 69));
	drawRect.moveRight(mY.geometry().left());
	painter.fillRect(drawRect, QColor(25, 161, 95));
}

QPointFBox::QPointFBox(QPointF vec)
	: mX(0.0f)
	, mY(0.0f)
{
	setValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX, 0, Qt::AlignLeft);
	h->addWidget(&mY, 0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::asValueChanged, this, &QPointFBox::emitValueChanged);
	connect(&mY, &QNumberBox::asValueChanged, this, &QPointFBox::emitValueChanged);
}

void QPointFBox::setValue(QPointF vec)
{
	blockSignals(true);
	mX.setVar(vec.x());
	mY.setVar(vec.y());
	blockSignals(false);
	emitValueChanged(QVariant());
}

QPointF QPointFBox::getValue()
{
	return QPointF(
		mX.getVar().toFloat(),
		mY.getVar().toFloat()
	);
}

void QPointFBox::emitValueChanged(QVariant)
{
	Q_EMIT asValueChanged(getValue());
}

void QPointFBox::paintEvent(QPaintEvent* event) {
	QWidget::paintEvent(event);
	QPainter painter(this);
	QRect drawRect(0, (height() - mX.height()) / 2, 4, mX.height());
	drawRect.moveRight(mX.geometry().left());
	painter.fillRect(drawRect, QColor(221, 81, 69));
	drawRect.moveRight(mY.geometry().left());
	painter.fillRect(drawRect, QColor(25, 161, 95));
}