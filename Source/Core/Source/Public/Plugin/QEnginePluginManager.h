#ifndef QEnginePluginManager_h__
#define QEnginePluginManager_h__

#include <QSharedPointer>
#include <QMap>
#include <QFileInfo>
#include <QLibrary>
#include "IEnginePlugin.h"
#include "QEngineCoreAPI.h"


struct QENGINECORE_API QEnginePluginHandler {
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

class QENGINECORE_API QEnginePluginManager: public QObject{
	Q_OBJECT
public:
	static QEnginePluginManager& Get();
	static void TearDown();
	void loadPlugin(QFileInfo fileInfo);
	void loadPlugins();
	QMap<QString, QEnginePluginHandler>& getPluginMap();
Q_SIGNALS:
	void asPluginChanged();
private:
	static QSharedPointer<QEnginePluginManager>& GetSingleton();
private:
	QMap<QString, QEnginePluginHandler> mPluginMap;
};

#endif // QEnginePluginManager_h__
