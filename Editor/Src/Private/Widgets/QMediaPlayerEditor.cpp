#include "Widgets/QMediaPlayerEditor.h"
#include "QBoxLayout"
#include "qevent.h"

void QMediaPlayerProgressBox::UpdateProgress(qint64 InProgress) {
	mProgress = InProgress;
	update();
}

void QMediaPlayerProgressBox::SetDuration(qint64 InDuration) {
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
			Q_EMIT AsProgressChangedByDrag(newProgress);
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
	CreateUI();
}

void QMediaPlayerEditor::SetupPlayer(QMediaPlayer* InPlayer) {
	mPlayer = InPlayer;
	mProgressBox->disconnect();
	mProgressBox->UpdateProgress(mPlayer->position());
	mProgressBox->SetDuration(mPlayer->duration());
	mSourceBox->SetFilePath(mPlayer->source().toString());
	OnPlaybackStateChanged(mPlayer->playbackState());
	connect(mPlayer, &QMediaPlayer::positionChanged, this, &QMediaPlayerEditor::OnProgressChanged);
	connect(mPlayer, &QMediaPlayer::durationChanged, this, &QMediaPlayerEditor::OnDurationChanged);
	connect(mPlayer, &QMediaPlayer::playbackStateChanged, this, &QMediaPlayerEditor::OnPlaybackStateChanged);
	connect(mProgressBox, &QMediaPlayerProgressBox::AsProgressChangedByDrag, this, [this](qint64 InProgress) {
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
	connect(mSourceBox, &QFilePathBox::AsPathChanged, this, [this](QString InPath) {
		mPlayer->setSource(QUrl::fromLocalFile(InPath));
	});
}

void QMediaPlayerEditor::CreateUI() {
	mSourceBox = new QFilePathBox();
	mProgressBox = new QMediaPlayerProgressBox;
	mProgressLable = new QElideLabel;
	mBtPlay = new QSvgButton();
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

void QMediaPlayerEditor::OnDurationChanged(qint64 InDuration) {
	mProgressBox->SetDuration(InDuration);
	OnRefreshProgressLabel(mPlayer->position(),InDuration);
}

void QMediaPlayerEditor::OnProgressChanged(qint64 InProgress) {
	mProgressBox->UpdateProgress(InProgress);
	OnRefreshProgressLabel(InProgress,mPlayer->duration());
}

void QMediaPlayerEditor::OnRefreshProgressLabel(qint64 InProgress, qint64 InDuration) {
	mProgressLable->SetText(QString("%1 / %2")
		.arg(ConvertMsToString(InProgress))
		.arg(ConvertMsToString(InDuration)));
}

void QMediaPlayerEditor::OnPlaybackStateChanged(QMediaPlayer::PlaybackState InState) {
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

