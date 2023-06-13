#include "Plugin/QEnginePluginManager.h"

QEnginePluginManager& QEnginePluginManager::Get() {
	return *GetSingleton().get();
}

void QEnginePluginManager::TearDown() {
	GetSingleton().reset();
}

QSharedPointer<QEnginePluginManager>& QEnginePluginManager::GetSingleton() {
	static QSharedPointer<QEnginePluginManager> Singleton(new QEnginePluginManager);
	return Singleton;
}

