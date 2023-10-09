#include "Widgets/QHoverTextEdit.h"
#include "QBoxLayout"

QHoverTextEdit::QHoverTextEdit(QString inText)
	: mTextEdit(new QTextEdit(inText))
{
	connect(mTextEdit, &QTextEdit::textChanged, this, &QHoverTextEdit::asTextChanged);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->addWidget(mTextEdit);
	h->setContentsMargins(2, 2, 2, 2);
}

void QHoverTextEdit::setDisplayText(QString inText)
{
	mTextEdit->setText(inText);
}

QString QHoverTextEdit::getDisplayText() const
{
	return mTextEdit->toPlainText();
}

QColor QHoverTextEdit::getPlaceholdColor() const
{
	return mPlaceholdColor;
}

void QHoverTextEdit::setPlaceholdColor(QColor val)
{
	mPlaceholdColor = val;
}

QString QHoverTextEdit::getPlaceholdText() const
{
	return mPlaceholdText;
}

void QHoverTextEdit::setPlaceholdText(QString val)
{
	mPlaceholdText = val;
}
