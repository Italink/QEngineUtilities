#ifndef QNotification_h__
#define QNotification_h__

#include "QWidget"
#include "QHash"
#include "QAbstractAnimation"
#include "QEngineEditorAPI.h"

class QNotificationBlock;

class QENGINEEDITOR_API QNotificationAnimation : public QAbstractAnimation {
public:
	QNotificationAnimation();
	void addNewBlock(QNotificationBlock* inBlock);
	void start();
protected:
	virtual int duration() const override { return 200; }
	virtual void updateCurrentTime(int currentTime) override;
	void updateCacheGeomtry();
private:
	QList<QNotificationBlock*> mNewBlockList;
	QHash<QNotificationBlock*,QRect> mCacheDestGeomtry;
	const int BlockFixedWidth = 200;
	const int BlockSpacing = 5;
};

class QENGINEEDITOR_API QNotification {
	friend class QNotificationAnimation;
	friend class QNotificationBlock;
public:
	static QNotification* Instance();
	static void ShowMessage(const QString& inTitle, const QString& inContent, float inDurationMs);
private:
	static void PushNewBlock(QNotificationBlock* inBlock);

	void pushBlock(QNotificationBlock* inBlock);
	void popBlock(QNotificationBlock* inBlock);
	QNotification() {};
	QList<QNotificationBlock*> mBlockList;
	QNotificationAnimation mAnimation;
};

class QENGINEEDITOR_API QNotificationBlock: public QWidget {
	friend class QNotification;
	Q_OBJECT
public:
	QNotificationBlock();
	using QWidget::QWidget;
private:
	virtual void active() = 0;
protected:
	void requestObsolete();
};

class QENGINEEDITOR_API QNotificationBlock_Message : public QNotificationBlock {
public:
	QNotificationBlock_Message(const QString& inTitle,const QString& inContent ,float inDuration);
private:
	virtual void active() override;
	int mDuration = 0;
};

#endif // QNotification_h__
