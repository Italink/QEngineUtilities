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
	IEnginePlugin::Info info;

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
	QList<QEnginePluginHandler>& getPluginHandlers();
Q_SIGNALS:
	void asPluginChanged();
private:
	static QSharedPointer<QEnginePluginManager>& GetSingleton();
private:
	QList<QEnginePluginHandler> mPluginHandlers;
};

#endif // QEnginePluginManager_h__
