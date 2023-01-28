#include "Widgets/QSvgButton.h"

QSvgButton::QSvgButton(QString inPath)
	:mIcon(inPath)
{
	//setFocusPolicy(Qt::NoFocus);
	mIcon.setUpdateCallBack([this]() {
		setIcon(mIcon.getIcon());
	});
}
