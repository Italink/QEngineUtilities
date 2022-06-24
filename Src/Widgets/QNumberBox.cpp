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
	mLbName = new QLabel;
	mLeValue = new QLineEdit_HasFocusSignal;
	mLbArrow = new QLabel;
	mIconArrow = new QSvgIcon(":/Resources/Icons/box_arrow.png");
	QHBoxLayout* h = new QHBoxLayout(this);
	h->setContentsMargins(0, 0, 0, 0);
	h->addWidget(mLbName);
	h->addWidget(mLeValue);
	h->addWidget(mLbArrow);
	mLbName->setContentsMargins(2, 0, 0, 2);
	mLbName->setAlignment(Qt::AlignCenter);
	mLbName->setFixedHeight(height());
	if (mLbName->text().isEmpty())
		mLbName->setVisible(false);
	mLbArrow->setFixedSize(height(), height());
	mIconArrow->setUpdateCallBack([this]() {
		mLbArrow->setPixmap(mIconArrow->getIcon().pixmap(mLbArrow->size()));
	});

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

void QNumberBox::SetHoverd(bool hoverd) {
	if (mHoverd != hoverd) {
		mHoverd = hoverd;
		update();
	}
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
}

void QNumberBox::enterEvent(QEnterEvent* event) {
	QWidget::enterEvent(event);
	SetHoverd(true);
}

void QNumberBox::leaveEvent(QEvent* event) {
	QWidget::leaveEvent(event);
	SetHoverd(false);
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
	QWidget::paintEvent(event);
	QPainter painter(this);
	if (mNumberAdaptor->GetFactor() > 0) {
		QRect overRect = mLeValue->geometry().adjusted(1, 1, -1, -1);
		overRect.setWidth(overRect.width() * mNumberAdaptor->GetFactor());
		painter.fillRect(overRect, QColor(100, 100, 100, 50));
	}
	if (mHoverd) {
		painter.setPen(QPen(QColor(79, 110, 242), 1));
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(rect().adjusted(1,1,-1,-1));
	}
}

QSize QNumberBox::sizeHint() const
{
	return { 60,30 };
}

void QNumberBox::focusOutEvent(QFocusEvent* event) {
	QWidget::focusOutEvent(event);
	SetHoverd(false);
}

