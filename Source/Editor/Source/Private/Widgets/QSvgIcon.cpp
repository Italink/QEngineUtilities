#include "Widgets/QSvgIcon.h"
#include <QPainter>


QSvgIcon::QSvgIcon(QString path, QString category, QColor initialColor)
	: mCategory(category)
	, mPath(path)
	, mColor(initialColor)
{
	IconMap[mCategory]<< this;
	updateIcon();

}

QSvgIcon::~QSvgIcon() {
	IconMap[mCategory].removeOne(this);
}

const QIcon& QSvgIcon::getIcon() {
	return mIcon;
}

const QIcon& QSvgIcon::getIcon() const {
	return mIcon;
}

void QSvgIcon::setUpdateCallBack(IconUpdateCallBack callback) {
	mCallBack = callback;
	if (mCallBack)
		mCallBack();
}

void QSvgIcon::setPath(QString path) {
	mPath = path;
	updateIcon();
}

QColor QSvgIcon::getColor() const {
	return mColor;
}

void QSvgIcon::setColor(QColor val) {
	mColor = val;
	updateIcon();
}

void QSvgIcon::setIconColor(QString category, QColor color)
{
	for (auto icon : IconMap[category]) {
		icon->setColor(color);
	}
}

void QSvgIcon::updateIcon() {
	QPixmap pixmap(mPath);
	if (pixmap.isNull())
		return;
	QPainter painter(&pixmap);
	painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
	painter.fillRect(pixmap.rect(), mColor);
	painter.end();
	mIcon = pixmap;
	if (mCallBack)
		mCallBack();
}