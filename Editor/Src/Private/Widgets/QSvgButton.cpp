#include "Widgets/QSvgButton.h"

QSvgButton::QSvgButton(QString inPath)
	:mIcon(inPath)
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
