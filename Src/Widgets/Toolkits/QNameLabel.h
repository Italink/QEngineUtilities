#ifndef QNameLabel_h__
#define QNameLabel_h__

#include "QWidget"

class QHoverLineEdit;

class QNameLabel :public QWidget {
	Q_OBJECT
public:
	using QWidget = QWidget;

	QNameLabel(const QString& inText);
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

#endif // QNameLabel_h__
