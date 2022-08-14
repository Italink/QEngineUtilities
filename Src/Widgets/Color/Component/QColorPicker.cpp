#include "QColorPicker.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>

QColor QColorPicker::Pick()
{
	QColorPicker picker;
	return picker.PickColor();
}

QColorPicker::QColorPicker()
	: QDialog()
{
	setFocusPolicy(Qt::NoFocus);
	setWindowFlag(Qt::FramelessWindowHint);
	setGeometry(QApplication::primaryScreen()->geometry());
	setCursor(QCursor(QPixmap(":/Resource/pen.png"), 0, 19));
	setMouseTracking(true);
}

void QColorPicker::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.fillRect(rect(), mScreen);
	QRect fdj(mMousePos.x(), mMousePos.y(), 100, 100);
	painter.drawPixmap(fdj, mScreen.copy(mMousePos.x() - 10, mMousePos.y() - 10, 20, 20).scaled(100, 100));
	painter.fillRect(fdj.adjusted(0, 70, 0, 0), Qt::black);
	QColor color = mScreen.toImage().pixel(mMousePos);
	painter.fillRect(fdj.adjusted(5, 75, -75, -5), color);
	painter.setPen(Qt::white);
	painter.drawText(fdj.adjusted(30, 75, -5, -5), Qt::AlignCenter, QString().asprintf("%03d,%03d,%03d", color.red(), color.green(), color.blue()));
	painter.setPen(QColor(0, 0, 0));
	painter.drawRect(rect().adjusted(1, 1, -1, -1));
}

void QColorPicker::mousePressEvent(QMouseEvent* e)
{
	QWidget::mousePressEvent(e);
	if (e->button() == Qt::LeftButton) {
		mCurrentColor = (mScreen.toImage().pixel(e->pos()));
		accept();
	}
	else {
		mCurrentColor = QColor(QColor::Invalid);
		reject();
	}
}

void QColorPicker::mouseMoveEvent(QMouseEvent* e)
{
	mMousePos = QCursor::pos();
	update();
}

QColor QColorPicker::PickColor()
{
	mMousePos = QCursor::pos();
	mScreen = QApplication::primaryScreen()->grabWindow(0);
	update();
	exec();
	return mCurrentColor;
}