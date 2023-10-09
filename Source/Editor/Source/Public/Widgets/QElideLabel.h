#ifndef QElideLabel_h__
#define QElideLabel_h__

#include "QWidget"
#include "QEngineEditorAPI.h"

class QHoverLineEdit;

class QENGINEEDITOR_API QElideLabel :public QWidget {
	Q_OBJECT
public:
	using QWidget = QWidget;

	QElideLabel(const QString& inText = QString() );
	QString getDisplayText() const { return mText; }
	void setDisplayText(QString val) { mText = val; update(); }
	void showRenameEditor();
	Q_SIGNAL void asNameChanged(QString);
private:
	QString mText;
	QHoverLineEdit* mLineEditor;
protected:
	virtual void paintEvent(QPaintEvent* event) override;
};

#endif // QElideLabel_h__
