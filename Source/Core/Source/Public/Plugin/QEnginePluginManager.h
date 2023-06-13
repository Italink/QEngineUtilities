#ifndef QEnginePluginManager_h__
#define QEnginePluginManager_h__

#include <QSharedPointer>
#include <QMap>
#include <QLibrary>
#include "IEnginePlugin.h"
#include "QEngineCoreAPI.h"


struct QENGINECORE_API QEnginePluginInfo {
	QString type;
	QSharedPointer<QLibrary> handle;
	QSharedPointer<IEnginePlugin> plugin;
};

class QENGINECORE_API QEnginePluginManager {
public:
	static QEnginePluginManager& Get();
	static void TearDown();

	void loadConfig(const QString& inConfigJsonFilename);

	void startupPlugins(const QString& inType);
	void shutdownPlugins(const QString& inType);

	void startupPlugin(const QString& inName);
	void shutdownPlugin(const QString& inName);
private:
	static QSharedPointer<QEnginePluginManager>& GetSingleton();
private:
	QMap<QString, QEnginePluginInfo> mPluginMap;
};

#endif // QEnginePluginManager_h__
