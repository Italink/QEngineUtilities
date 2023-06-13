#ifndef QHoverLineEdit_h__
#define QHoverLineEdit_h__

#include "QHoverWidget.h"
#include "QLineEdit"
#include "QEngineEditorAPI.h"

class QLineEditOverride;

class QENGINEEDITOR_API QHoverLineEdit : public QHoverWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor PlaceholdColor READ GetPlaceholdColor WRITE SetPlaceholdColor)
public:
	QHoverLineEdit(QString inText = "");

	void SetText(QString inText);
	QString GetText() const;

	QColor GetPlaceholdColor() const;
	void SetPlaceholdColor(QColor val);
	
	QString GetPlaceholdText() const;
	void SetPlaceholdText(QString val);

	QLineEdit* GetQLineEdit() const;
Q_SIGNALS:
	void AsTextChanged(QString);
	void AsEditingFinished();
private:
	QLineEditOverride* mLineEdit = nullptr;
};


#endif // QHoverLineEdit_h__
