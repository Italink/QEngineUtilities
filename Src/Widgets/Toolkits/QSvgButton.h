#ifndef QSvgButton_h__
#define QSvgButton_h__

#include "QPushButton"
#include "QSvgIcon.h"

class QSvgButton : public QPushButton {
public:
	QSvgButton(QString inPath);
private:
	QSvgIcon mIcon;
};

#endif // QSvgButton_h__
