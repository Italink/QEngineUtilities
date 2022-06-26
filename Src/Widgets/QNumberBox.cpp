#include "QNumberBox.h"
#include "QHBoxLayout"
#include "QPainter"
#include "QStyleOption"
#include "qevent.h"

void QNumberBox::CreateUI(){
	setAttribute(Qt::WA_StyledBackground);
	setFixedHeight(20);
	setStyleSheet("QLineEdit{background-color:transparent; font-size:11px;}");
	resize(60, 30);
	mLeValue = new QLineEdit_HasFocusSignal;
	mLbArrow = new QLabel;
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setContentsMargins(0, 0, 0, 0);
	h->setSpacing(0);
	h->addWidget(mLeValue);
	h->addWidget(mLbArrow);
	mLbArrow->setFixedSize(2, height());
	mLbArrow->setCursor(Qt::CursorShape::SizeHorCursor);
	mLeValue->setFixedHeight(height());
	mLeValue->setFrame(QFrame::NoFrame);
	mLeValue->setValidator(new QDoubleValidator);
	mLeValue->setAlignment(Qt::AlignLeft);
	mLeValue->setMinimumWidth(50);
	mLeValue->setText(mNumberAdaptor->GetText());
	SetEditEnabled(false);
}

void QNumberBox::ConnectUI()
{
	connect(mLeValue, &QLineEdit_HasFocusSignal::loseFocus, this, [this]() {
		SetEditEnabled(false);
	});
	connect(mLeValue, &QLineEdit::editingFinished, this, [this]() {
		QVariant lastVar = mNumberAdaptor->GetVar();
		mNumberAdaptor->SetText(mLeValue->text());
		if (mNumberAdaptor->GetText() != mLeValue->text()) {
			mLeValue->setText(mNumberAdaptor->GetText());
		}
		else{
			QVariant currentVar = mNumberAdaptor->GetVar();
			if (lastVar != currentVar) {
				Q_EMIT AsValueChanged(currentVar);
			}
		}
	});
}

void QNumberBox::SetEditEnabled(bool enable)
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

bool QNumberBox::GetEditEnabled()
{
	return mLeValue->focusPolicy() == Qt::FocusPolicy::StrongFocus;
}

QString QNumberBox::GetText()
{
	return mLeValue->text();
}

QVariant QNumberBox::GetVar()
{
	return mNumberAdaptor->GetVar();
}

void QNumberBox::SetVar(QVariant var)
{
	mNumberAdaptor->SetVar(var);
	mLeValue->setText(mNumberAdaptor->GetText());
}

void QNumberBox::mousePressEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		mClickPosition = event->pos();
		if (GetEditEnabled() && mLbArrow->geometry().contains(event->pos())) {
			SetEditEnabled(false);
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
		else if (mClickPosition == event->pos() && !GetEditEnabled() && this->cursor() != Qt::BlankCursor) {
			SetEditEnabled(true);
		}
	}
}

void QNumberBox::mouseMoveEvent(QMouseEvent* event)
{
	if (!GetEditEnabled() && event->buttons() & Qt::LeftButton) {
		setCursor(Qt::BlankCursor);
		mLbArrow->setCursor(Qt::CursorShape::BlankCursor);
		QPointF offset = event->position() - mClickPosition;
		QVariant lastVar = mNumberAdaptor->GetVar();
		mNumberAdaptor->MoveOffset(offset);
		QVariant currentVar = mNumberAdaptor->GetVar();
		if (lastVar!= currentVar) {
			Q_EMIT AsValueChanged(mNumberAdaptor->GetVar());
		}
		mLeValue->setText(mNumberAdaptor->GetText());
		QCursor::setPos(mapToGlobal(mClickPosition.toPoint()));
	}
}

void QNumberBox::paintEvent(QPaintEvent* event)
{
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	if (mNumberAdaptor->GetLimitedFactor() > 0) {
		QRect overRect = rect().adjusted(3, 3, -3, -3);
		overRect.setWidth(overRect.width() * mNumberAdaptor->GetLimitedFactor());
		painter.fillRect(overRect, mHoverd ? mHoverColor : QColor(100, 100, 100, 50));
	}
}

QSize QNumberBox::sizeHint() const
{
	return { 60,30 };
}
