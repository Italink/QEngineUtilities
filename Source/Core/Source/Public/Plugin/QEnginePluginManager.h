#ifndef QEnginePluginManager_h__
#define QEnginePluginManager_h__

#include <QSharedPointer>
#include <QMap>
#include <QLibrary>
#include "IEnginePlugin.h"
#include "QEngineCoreAPI.h"


struct QENGINECORE_API QEnginePluginInfo {
	bool bAlreadyStarted = false;
	QSharedPointer<QLibrary> handle;
	QSharedPointer<IEnginePlugin> plugin;

	void startup() {
		if (!bAlreadyStarted) {
			bAlreadyStarted = true;
			plugin->startup();
		}
	}

	void shutdown() {
		if (bAlreadyStarted) {
			plugin->shutdown();
			bAlreadyStarted = false;
		}
	}
};

class QENGINECORE_API QEnginePluginManager {
public:
	static QEnginePluginManager& Get();
	static void TearDown();

	void loadPlugins();

	QMap<QString, QEnginePluginInfo>& getPluginMap();
private:
	static QSharedPointer<QEnginePluginManager>& GetSingleton();
private:
	QMap<QString, QEnginePluginInfo> mPluginMap;
};

#endif // QEnginePluginManager_h__
