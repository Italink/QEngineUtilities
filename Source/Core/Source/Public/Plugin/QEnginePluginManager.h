#ifndef QEnginePluginManager_h__
#define QEnginePluginManager_h__

#include <QSharedPointer>
#include <QMap>
#include "IEnginePlugin.h"
#include "QEngineCoreAPI.h"

struct QENGINECORE_API QEnginePluginInfo {
	QString PluginName;
	QString Filename;
	void* Handle;
	QScopedPointer<IEnginePlugin> Plugin;
};

class QENGINECORE_API QEnginePluginManager {
public:
	static QEnginePluginManager& Get();
	static void TearDown();

	void registerPlugin(const QString& inName);
	void unregisterPlugin(const QString& inName);

	void startupPlugin(const QString& inName);
	void shutdownPlugin(const QString& inName);
private:
	static QSharedPointer<QEnginePluginManager>& GetSingleton();
private:
	QMap<QString, QEnginePluginInfo> mPluginMap;
};


#endif // QEnginePluginManager_h__
