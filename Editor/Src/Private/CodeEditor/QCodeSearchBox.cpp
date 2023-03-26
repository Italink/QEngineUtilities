#include <QLineEdit>
#include <QHBoxLayout>
#include <QKeyEvent>
#include "CodeEditor/QCodeSearchBox.h"
#include "Widgets/QSvgButton.h"

QCodeSearchBox::QCodeSearchBox()
	: mSearchEdit(new QLineEdit)
	, btCaseSensitive(new QSvgButton((":/Resources/case.png")))
	, btWholeMatching(new QSvgButton((":/Resources/text-format.png")))
	, btUseRegExp(new QSvgButton((":/Resources/ic_exp.png")))
	, btLast(new QSvgButton((":/Resources/arrow-left-l.png")))
	, btNext(new QSvgButton((":/Resources/arrow-right-l.png")))
	, btReplaceNext(new QSvgButton((":/Resources/replace.png")))
	, btClose(new QSvgButton((":/Resources/close.png")))
	, mReplaceEdit(new QLineEdit)
{
	setFixedSize(250, 80);
	setAttribute(Qt::WA_StyledBackground);
	QVBoxLayout* v = new QVBoxLayout(this);

	QHBoxLayout* h = new QHBoxLayout();
	h->setContentsMargins(0, 0, 0, 0);
	h->addWidget(mSearchEdit);
	h->addWidget(btNext);
	h->addWidget(btClose);
	v->addLayout(h, 0);

	h = new QHBoxLayout();
	h->setContentsMargins(0, 0, 0, 0);
	h->addWidget(mReplaceEdit, 0);
	h->addWidget(btReplaceNext);
	v->addLayout(h);

	h = new QHBoxLayout();
	h->setContentsMargins(2, 2, 2, 2);
	h->setAlignment(Qt::AlignLeft);
	h->addWidget(btCaseSensitive);
	h->addWidget(btWholeMatching);
	h->addWidget(btUseRegExp);
	v->addLayout(h, 0);

	mSearchEdit->setPlaceholderText("Search ...");
	mSearchEdit->setFixedHeight(20);
	mReplaceEdit->setPlaceholderText("Replace ...");
	mReplaceEdit->setFixedHeight(20);

	btCaseSensitive->setCheckable(true);
	btWholeMatching->setCheckable(true);
	btUseRegExp->setCheckable(true);

	btClose->setFixedSize(20, 20);
	btCaseSensitive->setFixedSize(20, 20);
	btWholeMatching->setFixedSize(20, 20);
	btUseRegExp->setFixedSize(20, 20);
	btLast->setFixedSize(20, 20);
	btNext->setFixedSize(20, 20);
	btReplaceNext->setFixedSize(20, 20);

	//btCaseSensitive->SET(Qt::white);
	//btCaseSensitive->setCheckColor(Qt::white);

	//btWholeMatching->setHoverColor(Qt::white);
	//btWholeMatching->setCheckColor(Qt::white);

	//btUseRegExp->setHoverColor(Qt::white);
	//btUseRegExp->setCheckColor(Qt::white);

	//btLast->setHoverColor(Qt::white);
	//btLast->setCheckColor(Qt::white);

	//btNext->setHoverColor(Qt::white);
	//btNext->setCheckColor(Qt::white);

	//btClose->setHoverColor(Qt::white);

	//btReplaceNext->setHoverColor(Qt::white);
	//btReplaceNext->setCheckColor(Qt::white);

	connect(mSearchEdit, &QLineEdit::returnPressed, this, [this]() {
		if (mSearchEdit->text().isEmpty())
			return;
		Q_EMIT requestSearch(getCurrentContext());
	});

	connect(btNext, &QPushButton::clicked, this, [this]() {
		if (mSearchEdit->text().isEmpty())
			return;
		Q_EMIT requestSearch(getCurrentContext());
	});

	connect(btReplaceNext, &QPushButton::clicked, this, [this]() {
		if (mSearchEdit->text().isEmpty() || mReplaceEdit->text().isEmpty())
			return;
		Q_EMIT requestReplace(getCurrentContext(), mReplaceEdit->text());
	});
	connect(btClose, &QPushButton::clicked, this, [this]() {
		close();
	});
}

void QCodeSearchBox::showSearch(const QString& str /*= QString()*/)
{
	mSearchEdit->setText(str);
	show();
	mSearchEdit->setFocus();
}

void QCodeSearchBox::keyPressEvent(QKeyEvent* e)
{
	e->accept();
}

QCodeEditor::SearchContext QCodeSearchBox::getCurrentContext()
{
	QCodeEditor::SearchContext ctx;
	ctx.text = mSearchEdit->text();
	ctx.isCaseSensitive = btCaseSensitive->isChecked();
	ctx.isWholeMatching = btWholeMatching->isChecked();
	ctx.useRegularExpression = btUseRegExp->isChecked();
	return ctx;
}