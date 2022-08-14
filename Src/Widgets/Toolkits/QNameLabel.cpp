﻿#include "QNameLabel.h"
#include "QHoverLineEdit.h"
#include "QBoxLayout"
#include "QPainter"
#include "QFontMetrics"
#include "QStyleOption"

QNameLabel::QNameLabel(const QString& inText)
	: mText(inText)
	, mLineEditor(new QHoverLineEdit)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setContentsMargins(0, 0, 0, 0);
	h->addWidget(mLineEditor);
	mLineEditor->setVisible(false);
	connect(mLineEditor, &QHoverLineEdit::AsEditingFinished, this, [this]() {
		mText = mLineEditor->GetText();
		Q_EMIT AsNameChanged(mLineEditor->GetText());
		mLineEditor->setVisible(false);
	});
}

void QNameLabel::ShowRenameEditor() {
	mLineEditor->setVisible(true);
	mLineEditor->GetQLineEdit()->selectAll();
	mLineEditor->GetQLineEdit()->activateWindow();
	mLineEditor->GetQLineEdit()->setFocus();
}

void QNameLabel::paintEvent(QPaintEvent* event) {
	if (!mLineEditor->isVisible()) {
		QPainter painter(this);
		QStyleOption opt;
		opt.initFrom(this);
		painter.setPen(opt.palette.color(QPalette::Text));
		painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, opt.fontMetrics.elidedText(mText, Qt::ElideRight, width()));
	}
}

