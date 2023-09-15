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
	QString GetText() const { return mText; }
	void SetText(QString val) { mText = val; update(); }
	void ShowRenameEditor();
	Q_SIGNAL void AsNameChanged(QString);
private:
	QString mText;
	QHoverLineEdit* mLineEditor;
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual QSize sizeHint() const override;
};

#endif // QElideLabel_h__
