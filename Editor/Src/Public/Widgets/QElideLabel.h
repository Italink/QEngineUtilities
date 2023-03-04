#ifndef QElideLabel_h__
#define QElideLabel_h__

#include "QWidget"

class QHoverLineEdit;

class QElideLabel :public QWidget {
	Q_OBJECT
public:
	using QWidget = QWidget;

	QElideLabel(const QString& inText);
	QString GetText() const { return mText; }
	void SetText(QString val) { mText = val; }
	void ShowRenameEditor();
	Q_SIGNAL void AsNameChanged(QString);
private:
	QString mText;
	QHoverLineEdit* mLineEditor;
protected:
	virtual void paintEvent(QPaintEvent* event) override;
};

#endif // QElideLabel_h__