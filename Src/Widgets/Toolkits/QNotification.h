#ifndef QNotification_h__
#define QNotification_h__

#include "QWidget"
#include "QHash"
#include "QAbstractAnimation"

class QNotificationBlockInterface;

class QNotificationAnimation : public QAbstractAnimation {
public:
	QNotificationAnimation();
	void AddNewBlock(QNotificationBlockInterface* inBlock);
	void Start();
protected:
	virtual int duration() const override { return 200; }
	virtual void updateCurrentTime(int currentTime) override;
	void updateCacheGeomtry();
private:
	QList<QNotificationBlockInterface*> mNewBlockList;
	QHash<QNotificationBlockInterface*,QRect> mCacheDestGeomtry;
	const int BlockFixedWidth = 200;
	const int BlockSpacing = 5;
};

class QNotification {
	friend class QNotificationAnimation;
	friend class QNotificationBlockInterface;
public:
	static QNotification* Instance();
	static void ShowMessage(const QString& inTitle, const QString& inContent, float inDurationMs);
private:
	static void PushNewBlock(QNotificationBlockInterface* inBlock);

	void PushBlock(QNotificationBlockInterface* inBlock);
	void PopBlock(QNotificationBlockInterface* inBlock);
	QNotification() {};
	QList<QNotificationBlockInterface*> mBlockList;
	QNotificationAnimation mAnimation;
};

class QNotificationBlockInterface: public QWidget {
	friend class QNotification;
public:
	QNotificationBlockInterface();
private:
	virtual void Active() = 0;
protected:
	void RequestObsolete();
};

class QNotificationBlock_Message : public QNotificationBlockInterface {
public:
	QNotificationBlock_Message(const QString& inTitle,const QString& inContent ,float inDuration);
private:
	virtual void Active() override;
	int mDuration = 0;
};




#endif // QNotification_h__
