#ifndef QEnginePluginManager_h__
#define QEnginePluginManager_h__

#include <QSharedPointer>
#include <QMap>
#include <QFileInfo>
#include <QLibrary>
#include "QEngineObjectManager.h"

QEngineObjectManager& QEngineObjectManager::Get()
{
	static QEngineObjectManager Instance;
	return Instance;
}

void QEngineObjectManager::registerMetaObject(const QMetaObject* inMetaObject)
{
	mAllMetaObjects.insert(inMetaObject->className(), inMetaObject);
	while (inMetaObject->superClass()) {
		mDerivedMetaObjects[inMetaObject->superClass()] << inMetaObject;
		inMetaObject = inMetaObject->superClass();
	}
}

const QMetaObject* QEngineObjectManager::getMetaObjectByName(const QString& inName)
{
	return mAllMetaObjects.value(inName);
}

QList<const QMetaObject*> QEngineObjectManager::getDerivedMetaObjects(const QMetaObject* inMetaObject)
{
	return mDerivedMetaObjects.value(inMetaObject);
}

#endif