#ifndef QCodeSearchBox_h__
#define QCodeSearchBox_h__

#include <QWidget>
#include "QCodeEditor.h"

class QLineEdit;
class QSvgButton;

class QENGINEEDITOR_API QCodeSearchBox :public QWidget {
	Q_OBJECT
public:
	QCodeSearchBox();
	using QWidget::QWidget;
	void showSearch(const QString& str = QString());
protected:
	void keyPressEvent(QKeyEvent* e) override;
	QCodeEditor::SearchContext getCurrentContext();
Q_SIGNALS:
	void requestSearch(const QCodeEditor::SearchContext& ctx);
	void requestReplace(const QCodeEditor::SearchContext& ctx, const QString& dst);
private:
	QLineEdit* mSearchEdit;
	QSvgButton* btCaseSensitive;
	QSvgButton* btWholeMatching;
	QSvgButton* btUseRegExp;
	QSvgButton* btNext;
	QSvgButton* btLast;
	QSvgButton* btClose;
	QLineEdit* mReplaceEdit;
	QSvgButton* btReplaceNext;
};

#endif // QCodeSearchBox_h__
