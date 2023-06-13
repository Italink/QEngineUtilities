#ifndef QMediaPlayerEditor_h__
#define QMediaPlayerEditor_h__

#include "QHoverWidget.h"
#include "QMediaPlayer"
#include "QElideLabel.h"
#include "QSvgButton.h"
#include "QFilePathBox.h"

class QENGINEEDITOR_API QMediaPlayerProgressBox :public QHoverWidget {
	Q_OBJECT
public:
	QMediaPlayerProgressBox();
	void UpdateProgress(qint64 InProgress);
	void SetDuration(qint64 InDuration);
protected:
	void enterEvent(QEnterEvent* event) override;
	void leaveEvent(QEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
Q_SIGNALS:
	void AsProgressChangedByDrag(qint64);
private:
	qint64 mProgress = 0;
	qint64 mDuration = 0;
};

class QENGINEEDITOR_API QMediaPlayerEditor: public QWidget {
	Q_OBJECT
public:
	QMediaPlayerEditor();
	void SetupPlayer(QMediaPlayer* InPlayer);
protected:
	void CreateUI();
	void OnDurationChanged(qint64 InDuration);
	void OnProgressChanged(qint64 InProgress);
	void OnRefreshProgressLabel(qint64 InProgress, qint64 InDuration);
	void OnPlaybackStateChanged(QMediaPlayer::PlaybackState InState);
	static QString ConvertMsToString(qint64 InMs);
private:
	QMediaPlayer* mPlayer = nullptr;
	QFilePathBox* mSourceBox = nullptr;
	QMediaPlayerProgressBox* mProgressBox = nullptr;
	QElideLabel* mProgressLable = nullptr;
	QSvgButton* mBtPlay = nullptr;
};

#endif // QMediaPlayerEditor_h__
