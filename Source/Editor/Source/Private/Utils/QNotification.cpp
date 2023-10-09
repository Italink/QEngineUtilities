#include "Utils/QNotification.h"
#include "QVBoxLayout"
#include "QLabel"
#include "QApplication"
#include "QScreen"
#include "QPropertyAnimation"
#include "QTimer"

QNotification* QNotification::Instance()
{
	static QNotification Ins;
	return &Ins;
}

void QNotification::ShowMessage(const QString& inTitle, const QString& inContent, float inDurationMs)
{
	QNotification::PushNewBlock(new QNotificationBlock_Message(inTitle, inContent, inDurationMs));
}

void QNotification::PushNewBlock(QNotificationBlock* inBlock)
{
	QNotification::Instance()->mAnimation.addNewBlock(inBlock);
}

void QNotification::pushBlock(QNotificationBlock* inBlock)
{
	QPropertyAnimation* Anim = new QPropertyAnimation;
	Anim->setTargetObject(inBlock);
	Anim->setPropertyName("windowOpacity");
	Anim->setStartValue(0.0);
	Anim->setEndValue(1.0);
	Anim->setDuration(200);
	Anim->start(QAbstractAnimation::DeleteWhenStopped);
	QObject::connect(Anim, &QPropertyAnimation::finished, inBlock, [this, inBlock]() {
		mBlockList << inBlock;
		inBlock->active();
	});
}

void QNotification::popBlock(QNotificationBlock* inBlock)
{
	QPropertyAnimation* Anim = new QPropertyAnimation;
	Anim->setTargetObject(inBlock);
	Anim->setPropertyName("windowOpacity");
	Anim->setStartValue(1.0);
	Anim->setEndValue(0.0);
	Anim->setDuration(200);
	Anim->start(QAbstractAnimation::DeleteWhenStopped);
	QObject::connect(Anim, &QPropertyAnimation::finished, inBlock, [this, inBlock]() {
		mBlockList.removeOne(inBlock);
		inBlock->deleteLater();
		mAnimation.start();
	});
}

QNotificationAnimation::QNotificationAnimation()
{
	QObject::connect(this, &QAbstractAnimation::finished, [this]() {
		for (auto* Block : mNewBlockList) {
			QNotification::Instance()->pushBlock(Block);
		}
		mNewBlockList.clear();
	});
}

void QNotificationAnimation::addNewBlock(QNotificationBlock* inBlock)
{
	mNewBlockList << inBlock;
	inBlock->setFixedWidth(BlockFixedWidth);
	inBlock->show();
	inBlock->setWindowOpacity(0);
	start();
}

void QNotificationAnimation::start()
{
	updateCacheGeomtry();
	if (state() != QAbstractAnimation::Running) {
		start();
	}
}

template<typename T>
T interp(const T& from, const T& to, qreal progress) {
	return (from + (to - from) * progress);
}

QRect interp(const QRect& f, const QRect& t, qreal process) {
	QRect ret;
	ret.setCoords(interp<int>(f.left(), t.left(), process),
		interp<int>(f.top(), t.top(), process),
		interp<int>(f.right(), t.right(), process),
		interp<int>(f.bottom(), t.bottom(), process));
	return ret;
}

void QNotificationAnimation::updateCurrentTime(int currentTime)
{
	for (auto Iter = mCacheDestGeomtry.begin(); Iter != mCacheDestGeomtry.end(); ++Iter) {
		QRect CurRect = Iter.key()->geometry();
		QRect NextRect = interp(CurRect, Iter.value(), currentTime /(float) duration());
		Iter.key()->setGeometry(NextRect);
	}
}

void QNotificationAnimation::updateCacheGeomtry()
{
	mCacheDestGeomtry.clear();
	QNotification* Notification = QNotification::Instance();
	QRect ScreenRect = qApp->primaryScreen()->geometry();
	int HeightOffset = ScreenRect.bottom() - 50;
	for (auto WattingBlock : mNewBlockList) {
		QRect Rect(0, 0, BlockFixedWidth, WattingBlock->height());
		Rect.moveBottomRight(QPoint( ScreenRect.right() - BlockSpacing, HeightOffset - BlockSpacing));
		WattingBlock->setGeometry(Rect);
		HeightOffset -= WattingBlock->height() + BlockSpacing;
	}
	for (int i = Notification->mBlockList.size()-1; i >= 0 ; i--) {
		QRect Geomtry = Notification->mBlockList[i]->geometry();
		Geomtry.moveBottom(HeightOffset - BlockSpacing);
		mCacheDestGeomtry[Notification->mBlockList[i]] = Geomtry;
		HeightOffset -= Geomtry.height() + BlockSpacing;
	}
}

QNotificationBlock::QNotificationBlock()
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
	//setStyleSheet(QDetailWidgetStyleManager::Instance()->getStylesheet());
	//setAttribute(Qt::WA_TranslucentBackground);
}

void QNotificationBlock::requestObsolete()
{
	QNotification::Instance()->popBlock(this);
}

QNotificationBlock_Message::QNotificationBlock_Message(const QString& inTitle, const QString& inContent, float inDuration)
{
	QVBoxLayout* v = new QVBoxLayout(this);
	v->addWidget(new QLabel(inTitle));
	v->addWidget(new QLabel(inContent));
	mDuration = inDuration;
}

void QNotificationBlock_Message::active()
{
	QTimer::singleShot(mDuration, this, &QNotificationBlock_Message::requestObsolete);
}