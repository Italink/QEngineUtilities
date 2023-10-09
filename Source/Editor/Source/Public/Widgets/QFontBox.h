#ifndef QFontBox_h__
#define QFontBox_h__

#include <QFont>
#include "QHoverWidget.h"
#include "QNumberBox.h"
#include "QCheckBox"
#include "QEngineEditorAPI.h"

class QFontComboBox;

class QENGINEEDITOR_API QFontBox : public QWidget{
	Q_OBJECT
public:
	QFontBox(QFont value = QFont());

	QFont getFont();
	void setFont(const QFont& inFont);

	Q_SIGNAL void asFontChanged(const QFont&);
private:	
	QFontComboBox* mComboBox;
	QNumberBox* mPointSize;
	QNumberBox* mWeight;
	QCheckBox* mItalic;
};

#endif // QFontBox_h__
