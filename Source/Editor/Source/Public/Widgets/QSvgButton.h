#ifndef QSvgButton_h__
#define QSvgButton_h__

#include "QPushButton"
#include "QSvgIcon.h"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QSvgButton : public QPushButton {
public:
	QSvgButton(QString inPath, QString inIconCategory = QString());
	void setIconPath(QString inPath);
private:
	QSvgIcon mIcon;
};

#endif // QSvgButton_h__
