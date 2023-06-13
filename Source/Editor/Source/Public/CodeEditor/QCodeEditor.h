#ifndef QCodeEditor_h__
#define QCodeEditor_h__

#include <Qsci/qsciapis.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qsciscintilla.h>
#include "QEngineEditorAPI.h"

class QCodeSearchBox;

class QENGINEEDITOR_API QCodeEditor : public QsciScintilla
{
	Q_OBJECT
public:
	explicit QCodeEditor(QsciLexer* lexer, QWidget* parent = nullptr);
public:
	struct SearchContext {
		QString text;
		bool useRegularExpression = false;
		bool isCaseSensitive = false;
		bool isWholeMatching = false;
		bool isForward = false;
	};
	QsciAPIs* getApis() const { return mApis; }
	void setApis(QsciAPIs* val) { mApis = val; }
protected:
	void showEvent(QShowEvent* event) override;
	void resizeEvent(QResizeEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
	void searchCode(const SearchContext& sctx);
	void replaceCode(const SearchContext& sctx, const QString& dst);
protected:
	QsciLexer* mLexer;
	QsciAPIs* mApis;

	QCodeSearchBox* mSearchEditor;
};

#endif // QCodeEditor_h__
