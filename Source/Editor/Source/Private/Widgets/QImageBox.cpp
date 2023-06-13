#include "Widgets/QImageBox.h"

QImageBox::QImageBox(QImage value /*= QImage()*/) {
	setMinimumSize(60, 60);
	mImage = value;
}

QImage QImageBox::GetImage() {
	return mImage;
}

void QImageBox::SetImage(const QImage& inImage) {
	if (mImage != inImage) {
		mImage = inImage;
		Q_EMIT AsImageChanged(mImage);
	}
}

void QImageBox::OpenFileSelector() {

}

void QImageBox::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.drawImage(rect(), mImage);
}
