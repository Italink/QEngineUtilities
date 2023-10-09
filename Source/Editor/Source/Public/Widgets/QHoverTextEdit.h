#ifndef QHoverTextEdit_h__
#define QHoverTextEdit_h__

#include "QHoverWidget.h"
#include "QTextEdit"

class QENGINEEDITOR_API QHoverTextEdit : public QHoverWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor PlaceholdColor READ getPlaceholdColor WRITE setPlaceholdColor)
public:
	QHoverTextEdit(QString inText = "");

	void setDisplayText(QString inText);
	QString getDisplayText() const;

	QColor getPlaceholdColor() const;
	void setPlaceholdColor(QColor val);
	
	QString getPlaceholdText() const;
	void setPlaceholdText(QString val);

	QTextEdit* getQTextEdit() const { return mTextEdit; }
Q_SIGNALS:
	void asTextChanged();
private:
	QTextEdit* mTextEdit = nullptr;

	QColor mPlaceholdColor;
	QString mPlaceholdText;
};


#endif // QHoverTextEdit_h__
