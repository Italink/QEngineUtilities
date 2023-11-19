#include "Widgets/QMediaPlayerEditor.h"
#include "QBoxLayout"
#include "qevent.h"

void QMediaPlayerProgressBox::updateProgress(qint64 InProgress) {
	mProgress = InProgress;
	update();
}

void QMediaPlayerProgressBox::setDuration(qint64 InDuration) {
	mDuration = InDuration;
	update();
}

void QMediaPlayerProgressBox::enterEvent(QEnterEvent* event) {
	QHoverWidget::enterEvent(event);
	setCursor(Qt::CursorShape::SizeHorCursor);
}

void QMediaPlayerProgressBox::leaveEvent(QEvent* event) {
	QHoverWidget::leaveEvent(event);
	setCursor(Qt::CursorShape::ArrowCursor);
}

void QMediaPlayerProgressBox::mousePressEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		setCursor(Qt::CursorShape::PointingHandCursor);
	}
}

void QMediaPlayerProgressBox::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		setCursor(Qt::CursorShape::SizeHorCursor);
	}
}

void QMediaPlayerProgressBox::mouseMoveEvent(QMouseEvent* event) {
	if (event->buttons() & Qt::LeftButton) {
		qint64 newProgress = qBound((qint64)0, (qint64)(mDuration * event->position().x() /(double) width()), mDuration);
		if (newProgress != mProgress) {
			Q_EMIT asProgressChangedByDrag(newProgress);
		}
	}
}

void QMediaPlayerProgressBox::paintEvent(QPaintEvent* event) {
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	if (mProgress > 0 && mDuration != 0) {
		QRect overRect = rect().adjusted(3, 3, -3, -3);
		overRect.setWidth(overRect.width() * (mProgress/(double)mDuration));
		painter.fillRect(overRect, mHoverd ? mHoverColor : QColor(150, 150, 150, 100));
	}
}

QMediaPlayerProgressBox::QMediaPlayerProgressBox() {
	setFixedHeight(20);
}

QMediaPlayerEditor::QMediaPlayerEditor() {
	createUI();
}

void QMediaPlayerEditor::setupPlayer(QMediaPlayer* InPlayer) {
	mPlayer = InPlayer;
	mProgressBox->disconnect();
	mProgressBox->updateProgress(mPlayer->position());
	mProgressBox->setDuration(mPlayer->duration());
	mSourceBox->setFilePath(mPlayer->source().toString());
	onPlaybackStateChanged(mPlayer->playbackState());
	connect(mPlayer, &QMediaPlayer::positionChanged, this, &QMediaPlayerEditor::onProgressChanged);
	connect(mPlayer, &QMediaPlayer::durationChanged, this, &QMediaPlayerEditor::onDurationChanged);
	connect(mPlayer, &QMediaPlayer::playbackStateChanged, this, &QMediaPlayerEditor::onPlaybackStateChanged);
	connect(mProgressBox, &QMediaPlayerProgressBox::asProgressChangedByDrag, this, [this](qint64 InProgress) {
		mPlayer->pause();
		mPlayer->setPosition(InProgress);
	});
	connect(mBtPlay, &QPushButton::clicked, this, [this](){
		if (mPlayer->playbackState() == QMediaPlayer::PlayingState) {
			mPlayer->pause();
		}
		else if (mPlayer->playbackState() == QMediaPlayer::StoppedState) {
			mPlayer->setPosition(0);
			mPlayer->play();
		}
		else if (mPlayer->playbackState() == QMediaPlayer::PausedState) {
			mPlayer->play();
		}
	});
	connect(mSourceBox, &QFilePathBox::asPathChanged, this, [this](QString InPath) {
		mPlayer->setSource(QUrl::fromLocalFile(InPath));
	});
}

void QMediaPlayerEditor::createUI() {
	mSourceBox = new QFilePathBox();
	mProgressBox = new QMediaPlayerProgressBox;
	mProgressLable = new QElideLabel;
	mBtPlay = new QSvgButton(QString(),"DetailView");
	setMinimumHeight(60);
	mProgressLable->setFixedHeight(20);
	mBtPlay->setFixedSize(25,25);
	QVBoxLayout* vLayout = new QVBoxLayout(this);
	vLayout->setContentsMargins(0, 5, 6, 5);
	vLayout->setSpacing(2);
	vLayout->addWidget(mSourceBox);
	vLayout->addWidget(mProgressBox);
	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(3, 0, 0, 0);
	hLayout->addWidget(mProgressLable);
	hLayout->addWidget(mBtPlay);
	vLayout->addLayout(hLayout);
}

void QMediaPlayerEditor::onDurationChanged(qint64 InDuration) {
	mProgressBox->setDuration(InDuration);
	onRefreshProgressLabel(mPlayer->position(),InDuration);
}

void QMediaPlayerEditor::onProgressChanged(qint64 InProgress) {
	mProgressBox->updateProgress(InProgress);
	onRefreshProgressLabel(InProgress,mPlayer->duration());
}

void QMediaPlayerEditor::onRefreshProgressLabel(qint64 InProgress, qint64 InDuration) {
	mProgressLable->setDisplayText(QString("%1 / %2")
		.arg(ConvertMsToString(InProgress))
		.arg(ConvertMsToString(InDuration)));
}

void QMediaPlayerEditor::onPlaybackStateChanged(QMediaPlayer::PlaybackState InState) {
	if (InState == QMediaPlayer::PlaybackState::PlayingState) {
		mBtPlay->setIconPath(":/Resources/pause.png");
	}
	else{
		mBtPlay->setIconPath(":/Resources/play.png");
	}
}

QString QMediaPlayerEditor::ConvertMsToString(qint64 InMs) {
	return QString::asprintf("%02lld:%02lld", InMs / 1000 / 60, InMs / 1000 % 60);
}

