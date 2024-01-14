#ifndef QQmlHelper_h__
#define QQmlHelper_h__

#include <QObject>
#include <QVariant>
#include <QQuickItem>
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API QQmlHelper : public QObject {
	Q_OBJECT
public:
	static QQmlHelper* Get();

	Q_INVOKABLE QString numberToString(QVariant var,int precision);
	Q_INVOKABLE void setCursorPos(qreal x, qreal y);
	Q_INVOKABLE void setCursorPosTest(QQuickItem* item, qreal x, qreal y);
};

class QENGINEEDITOR_API QQmlLambdaHelper : public QObject {
	Q_OBJECT
		std::function<void()> mCallback;
public:
	QQmlLambdaHelper(std::function<void()>&& callback, QObject* parent)
		: QObject(parent)
		, mCallback(std::move(callback)) {}

	Q_SLOT void call() { mCallback(); }

	static QMetaObject::Connection connect(QObject* sender, const char* signal, std::function<void()>&& callback){
		if (!sender) 
			return {};
		return QObject::connect(sender, signal, new QQmlLambdaHelper(std::move(callback), sender), SLOT(call()));
	}
};

#endif // QQmlHelper_h__
