#ifndef QHoverLineEdit_h__
#define QHoverLineEdit_h__

#include "QHoverWidget.h"
#include "QLineEdit"
#include "QEngineEditorAPI.h"

class QLineEditOverride;

class QENGINEEDITOR_API QHoverLineEdit : public QHoverWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor PlaceholdColor READ getPlaceholdColor WRITE setPlaceholdColor)
public:
	QHoverLineEdit(QString inText = "");

	void setDisplayText(QString inText);
	QString getDisplayText() const;

	QColor getPlaceholdColor() const;
	void setPlaceholdColor(QColor val);
	
	QString getPlaceholdText() const;
	void setPlaceholdText(QString val);

	QLineEdit* GetQLineEdit() const;
Q_SIGNALS:
	void asTextChanged(QString);
	void AsEditingFinished();
private:
	QLineEditOverride* mLineEdit = nullptr;
};


#endif // QHoverLineEdit_h__
