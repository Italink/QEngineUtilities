#ifndef QEngineObjectManager_h__
#define QEngineObjectManager_h__

#include <QSharedPointer>
#include <QObject>
#include <QMap>
#include "QEngineCoreAPI.h"

class QENGINECORE_API QEngineObjectManager: public QObject{
	Q_OBJECT
public:
	static QEngineObjectManager& Get();
	void registerMetaObject(const QMetaObject* inMetaObject);
	const QMetaObject* getMetaObjectByName(const QString& inName);
	QList<const QMetaObject*> getDerivedMetaObjects(const QMetaObject* inMetaObject);
private:
	QMap<QString, const QMetaObject*> mAllMetaObjects;
	QMap<const QMetaObject*, QList<const QMetaObject*>> mDerivedMetaObjects;
};

#define QENGINE_REGISTER_CLASS(ClassName) \
	Q_DECLARE_METATYPE(ClassName) \
	Q_DECLARE_METATYPE(ClassName*) \
	struct ClassName##Register { \
		ClassName##Register() {  \
			qRegisterMetaType<ClassName>(); \
			qRegisterMetaType<ClassName*>(); \
			QEngineObjectManager::Get().registerMetaObject(&ClassName::staticMetaObject); \
		} \
	}; \
	static ClassName##Register ClassName##register;

#endif // QEngineObjectManager_h__
