#include "Widgets/QElideLabel.h"
#include "Widgets/QHoverLineEdit.h"
#include <QBoxLayout>
#include <QPainter>
#include <QFontMetrics>
#include <QStyleOption>

QElideLabel::QElideLabel(const QString& inText)
	: mText(inText)
	, mLineEditor(new QHoverLineEdit)
{
	this->setMinimumHeight(25);
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setContentsMargins(0, 0, 0, 0);
	h->addWidget(mLineEditor);
	mLineEditor->setVisible(false);
	connect(mLineEditor, &QHoverLineEdit::AsEditingFinished, this, [this]() {
		mText = mLineEditor->getDisplayText();
		Q_EMIT asNameChanged(mLineEditor->getDisplayText());
		mLineEditor->setVisible(false);
	});
}

void QElideLabel::showRenameEditor() {
	mLineEditor->setVisible(true);
	mLineEditor->GetQLineEdit()->selectAll();
	mLineEditor->GetQLineEdit()->activateWindow();
	mLineEditor->GetQLineEdit()->setFocus();
}

void QElideLabel::paintEvent(QPaintEvent* event) {
	if (!mLineEditor->isVisible()) {
		QPainter painter(this);
		QStyleOption opt;
		opt.initFrom(this);
		painter.setPen(opt.palette.color(QPalette::Text));
		painter.drawText(rect(), Qt::AlignLeft | Qt::AlignVCenter, opt.fontMetrics.elidedText(mText, Qt::ElideRight, width()));
	}
}
