#ifndef QFontBox_h__
#define QFontBox_h__

#include <QFont>
#include "QHoverWidget.h"
#include "QNumberBox.h"
#include "QCheckBox"

class QFontComboBox;

class QFontBox : public QWidget{
	Q_OBJECT
public:
	QFontBox(QFont value = QFont());

	QFont GetFont();
	void SetFont(const QFont& inFont);

	Q_SIGNAL void AsFontChanged(const QFont&);
private:	
	QFontComboBox* mComboBox;
	QNumberBox* mPointSize;
	QNumberBox* mWeight;
	QCheckBox* mItalic;
};

#endif // QFontBox_h__
