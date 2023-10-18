#include "Widgets/QNumberBox.h"
#include "QHBoxLayout"
#include "QPainter"
#include "qevent.h"
#include "QValidator"

void QNumberBox::createUI(){
	setAttribute(Qt::WA_StyledBackground);
	setFixedHeight(20);
	setStyleSheet("QLineEdit{background-color:transparent; font-size:11px;}");
	resize(60, 30);
	mLeValue = new QLineEdit_HasFocusSignal;
	mLbArrow = new QLabel;
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setContentsMargins(2, 2, 2, 2);
	h->setSpacing(0);
	h->addWidget(mLeValue);
	h->addWidget(mLbArrow);
	mLbArrow->setFixedSize(1, height());
	mLbArrow->setCursor(Qt::CursorShape::SizeHorCursor);
	mLeValue->setFixedHeight(height());
	mLeValue->setFrame(QFrame::NoFrame);
	mLeValue->setValidator(new QDoubleValidator);
	mLeValue->setAlignment(Qt::AlignLeft);
	mLeValue->setMinimumWidth(50);
	mLeValue->setText(mNumberAdaptor->getDisplayText());
	setEditEnabled(false);
}

void QNumberBox::connectUI()
{
	connect(mLeValue, &QLineEdit_HasFocusSignal::loseFocus, this, [this]() {
		setEditEnabled(false);
	});
	connect(mLeValue, &QLineEdit::editingFinished, this, [this]() {
		QVariant lastVar = mNumberAdaptor->getVar();
		mNumberAdaptor->setDisplayText(mLeValue->text());
		if (mNumberAdaptor->getDisplayText() != mLeValue->text()) {
			mLeValue->setText(mNumberAdaptor->getDisplayText());
		}
		else{
			QVariant currentVar = mNumberAdaptor->getVar();
			if (lastVar != currentVar) {
				Q_EMIT asValueChanged(currentVar);
			}
		}
	});
}

void QNumberBox::setEditEnabled(bool enable)
{
	if (enable) {
		mLeValue->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
		mLeValue->setAttribute(Qt::WA_TransparentForMouseEvents, false);
		setCursor(Qt::CursorShape::IBeamCursor);
		mLeValue->activateWindow();
		mLeValue->setFocus();
		mLeValue->setReadOnly(false);
		mLeValue->selectAll();
	}
	else {
		mLeValue->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		mLeValue->setAttribute(Qt::WA_TransparentForMouseEvents, true);
		setCursor(Qt::CursorShape::SizeHorCursor);
		mLeValue->setReadOnly(true);
	}
}

bool QNumberBox::getEditEnabled()
{
	return mLeValue->focusPolicy() == Qt::FocusPolicy::StrongFocus;
}

QString QNumberBox::getDisplayText()
{
	return mLeValue->text();
}

QVariant QNumberBox::getVar()
{
	return mNumberAdaptor->getVar();
}

void QNumberBox::setVar(QVariant var)
{
	mNumberAdaptor->setVar(var);
	mLeValue->setText(mNumberAdaptor->getDisplayText());
}

void QNumberBox::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		mClickPosition = event->globalPos();
		if (getEditEnabled() && mLbArrow->geometry().contains(event->pos())) {
			setEditEnabled(false);
		}
	}
}

void QNumberBox::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		if (this->cursor() == Qt::BlankCursor) {
			setCursor(Qt::CursorShape::SizeHorCursor);
			mLbArrow->setCursor(Qt::CursorShape::SizeHorCursor);
		}
		else if (mClickPosition == event->globalPos() && !getEditEnabled() && this->cursor() != Qt::BlankCursor) {
			setEditEnabled(true);
		}
	}
}

void QNumberBox::mouseMoveEvent(QMouseEvent* event)
{
	if (!getEditEnabled() && event->buttons() & Qt::LeftButton) {
		setCursor(Qt::BlankCursor);
		mLbArrow->setCursor(Qt::CursorShape::BlankCursor);
		QPointF offset = event->globalPos() - mClickPosition;
		QVariant lastVar = mNumberAdaptor->getVar();
		mNumberAdaptor->moveOffset(offset);
		QVariant currentVar = mNumberAdaptor->getVar();
		if (lastVar!= currentVar) {
			Q_EMIT asValueChanged(mNumberAdaptor->getVar());
		}
		mLeValue->setText(mNumberAdaptor->getDisplayText());
		QCursor::setPos(mClickPosition.toPoint());
	}
}

void QNumberBox::paintEvent(QPaintEvent* event)
{
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	if (mNumberAdaptor->getLimitedFactor() > 0) {
		QRect overRect = rect().adjusted(3, 3, -3, -3);
		overRect.setWidth(overRect.width() * mNumberAdaptor->getLimitedFactor());
		painter.fillRect(overRect, mHoverd ? mHoverColor : QColor(100, 100, 100, 50));
	}
}

QSize QNumberBox::sizeHint() const
{
	return { 60,30 };
}
