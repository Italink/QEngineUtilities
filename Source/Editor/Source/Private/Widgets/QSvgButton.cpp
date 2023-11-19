#include "Widgets/QSvgButton.h"

QSvgButton::QSvgButton(QString inPath, QString inIconCategory)
	:mIcon(inPath,inIconCategory)
{
	mIcon.setUpdateCallBack([this]() {
		setIcon(mIcon.getIcon());
	});
}

void QSvgButton::setIconPath(QString inPath) {
	if (!inPath.isEmpty()) {
		mIcon.setPath(inPath);
	}
}
