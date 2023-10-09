#include "Widgets/QImageBox.h"

QImageBox::QImageBox(QImage value /*= QImage()*/) {
	setMinimumSize(60, 60);
	mImage = value;
}

QImage QImageBox::getImage() {
	return mImage;
}

void QImageBox::setImage(const QImage& inImage) {
	if (mImage != inImage) {
		mImage = inImage;
		Q_EMIT asImageChanged(mImage);
	}
}

void QImageBox::openFileSelector() {

}

void QImageBox::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawImage(rect(), mImage);
}
