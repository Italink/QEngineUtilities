#include "Widgets/QVectorBox.h"
#include "QBoxLayout"
#include "QPainter"

QVector4DBox::QVector4DBox(QVector4D vec)
	: mX(0.0f)
	, mY(0.0f)
	, mZ(0.0f)
	, mW(0.0f)
{
	SetValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX,0, Qt::AlignLeft);
	h->addWidget(&mY,0, Qt::AlignLeft);
	h->addWidget(&mZ,0, Qt::AlignLeft);
	h->addWidget(&mW,0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::AsValueChanged, this, &QVector4DBox::EmitValueChanged);
	connect(&mY, &QNumberBox::AsValueChanged, this, &QVector4DBox::EmitValueChanged);
	connect(&mZ, &QNumberBox::AsValueChanged, this, &QVector4DBox::EmitValueChanged);
	connect(&mW, &QNumberBox::AsValueChanged, this, &QVector4DBox::EmitValueChanged);
}

void QVector4DBox::SetValue(QVector4D vec)
{
	blockSignals(true);
	mX.SetVar(vec.x());
	mY.SetVar(vec.y());
	mZ.SetVar(vec.z());
	mW.SetVar(vec.w());
	blockSignals(false);
	EmitValueChanged(QVariant());
}

QVector4D QVector4DBox::GetValue()
{
	return QVector4D(
		mX.GetVar().toFloat(),
		mY.GetVar().toFloat(),
		mZ.GetVar().toFloat(),
		mW.GetVar().toFloat()
	);
}

void QVector4DBox::EmitValueChanged(QVariant)
{
	Q_EMIT AsValueChanged(GetValue());
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
	SetValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX,0, Qt::AlignLeft);
	h->addWidget(&mY,0, Qt::AlignLeft);
	h->addWidget(&mZ,0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::AsValueChanged, this, &QVector3DBox::EmitValueChanged);
	connect(&mY, &QNumberBox::AsValueChanged, this, &QVector3DBox::EmitValueChanged);
	connect(&mZ, &QNumberBox::AsValueChanged, this, &QVector3DBox::EmitValueChanged);
}

void QVector3DBox::SetValue(QVector3D vec)
{
	blockSignals(true);
	mX.SetVar(vec.x());
	mY.SetVar(vec.y());
	mZ.SetVar(vec.z());
	blockSignals(false);
	EmitValueChanged(QVariant());
}

QVector3D QVector3DBox::GetValue()
{
	return QVector3D(
		mX.GetVar().toFloat(),
		mY.GetVar().toFloat(),
		mZ.GetVar().toFloat()
	);
}

void QVector3DBox::EmitValueChanged(QVariant)
{
	Q_EMIT AsValueChanged(GetValue());
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
	SetValue(vec);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setSpacing(5);
	h->setContentsMargins(4, 0, 0, 0);
	h->addWidget(&mX, 0, Qt::AlignLeft);
	h->addWidget(&mY ,0, Qt::AlignLeft);

	connect(&mX, &QNumberBox::AsValueChanged, this,&QVector2DBox::EmitValueChanged);
	connect(&mY, &QNumberBox::AsValueChanged, this,&QVector2DBox::EmitValueChanged);
}

void QVector2DBox::SetValue(QVector2D vec)
{
	blockSignals(true);
	mX.SetVar(vec.x());
	mY.SetVar(vec.y());
	blockSignals(false);
	EmitValueChanged(QVariant());
}

QVector2D QVector2DBox::GetValue()
{
	return QVector2D(
		mX.GetVar().toFloat(),
		mY.GetVar().toFloat()
	);
}

void QVector2DBox::EmitValueChanged(QVariant)
{
	Q_EMIT AsValueChanged(GetValue());
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