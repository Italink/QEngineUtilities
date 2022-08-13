#include "QHoverTextEdit.h"
#include "QBoxLayout"

QHoverTextEdit::QHoverTextEdit(QString inText)
	: mTextEdit(new QTextEdit(inText))
{
	connect(mTextEdit, &QTextEdit::textChanged, this, &QHoverTextEdit::AsTextChanged);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->addWidget(mTextEdit);
	h->setContentsMargins(2, 2, 2, 2);
}

void QHoverTextEdit::SetText(QString inText)
{
	mTextEdit->setText(inText);
}

QString QHoverTextEdit::GetText() const
{
	return mTextEdit->toPlainText();
}

QColor QHoverTextEdit::GetPlaceholdColor() const
{
	return mPlaceholdColor;
}

void QHoverTextEdit::SetPlaceholdColor(QColor val)
{
	mPlaceholdColor = val;
}

QString QHoverTextEdit::GetPlaceholdText() const
{
	return mPlaceholdText;
}

void QHoverTextEdit::SetPlaceholdText(QString val)
{
	mPlaceholdText = val;
}
