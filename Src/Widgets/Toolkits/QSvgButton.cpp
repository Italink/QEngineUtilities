#include "QSvgButton.h"

QSvgButton::QSvgButton(QString inPath)
	:mIcon(inPath)
{
	mIcon.setUpdateCallBack([this]() {
		setIcon(mIcon.getIcon());
	});
}
