#ifndef QHoverTextEdit_h__
#define QHoverTextEdit_h__

#include "QHoverWidget.h"
#include "QTextEdit"

class QHoverTextEdit : public QHoverWidget
{
	Q_OBJECT
		Q_PROPERTY(QColor PlaceholdColor READ GetPlaceholdColor WRITE SetPlaceholdColor)
public:
	QHoverTextEdit(QString inText = "");

	void SetText(QString inText);
	QString GetText() const;

	QColor GetPlaceholdColor() const;
	void SetPlaceholdColor(QColor val);
	
	QString GetPlaceholdText() const;
	void SetPlaceholdText(QString val);

	QTextEdit* GetQTextEdit() const { return mTextEdit; }
Q_SIGNALS:
	void AsTextChanged();
private:
	QTextEdit* mTextEdit = nullptr;

	QColor mPlaceholdColor;
	QString mPlaceholdText;
};


#endif // QHoverTextEdit_h__
