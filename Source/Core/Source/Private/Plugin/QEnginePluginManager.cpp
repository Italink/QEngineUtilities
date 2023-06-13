#include "Plugin/QEnginePluginManager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>

typedef IEnginePlugin* (*InitializePluginFunc)();

QEnginePluginManager& QEnginePluginManager::Get() {
	return *GetSingleton().get();
}

void QEnginePluginManager::TearDown() {
	GetSingleton().reset();
}

void QEnginePluginManager::loadConfig(const QString& inConfigJsonFilename) {
	QFile file(inConfigJsonFilename);
	file.open(QFile::ReadOnly);
	auto config = QJsonDocument::fromJson(file.readAll()).object();
	for (auto pluginName : config.keys()) {
		QEnginePluginInfo info;
		auto value = config[pluginName].toObject();
		info.type = value["Type"].toString();
		info.handle.reset(new QLibrary(pluginName + ".dll"));
		if (info.handle->load()) {
			InitializePluginFunc InitializePlugin = (InitializePluginFunc)info.handle->resolve("InitializePlugin");
			if (InitializePlugin) {
				info.plugin.reset(InitializePlugin());
				qDebug() << QString("Load %1 Success").arg(info.handle->fileName()).toLatin1().constData();
			}
			else {
				qDebug() << QString("Load %1 Failed").arg(info.handle->fileName()).toLatin1().constData();
			}
		}
		mPluginMap.insert(pluginName, info);
	}
}

void QEnginePluginManager::startupPlugins(const QString& inType) {
	for (const auto& plugin : mPluginMap) {
		if (inType.contains(plugin.type) && plugin.plugin != nullptr) {
			plugin.plugin->startup();
		}
	}
}

void QEnginePluginManager::shutdownPlugins(const QString& inType) {
	for (const auto& plugin : mPluginMap) {
		if (inType.contains(plugin.type) && plugin.plugin != nullptr) {
			plugin.plugin->shutdown();
		}
	}
}

void QEnginePluginManager::startupPlugin(const QString& inName) {
	if (mPluginMap.contains(inName)) {
		mPluginMap.value(inName).plugin->startup();
	}
}

void QEnginePluginManager::shutdownPlugin(const QString& inName) {
	if (mPluginMap.contains(inName)) {
		mPluginMap.value(inName).plugin->shutdown();
	}
}

QSharedPointer<QEnginePluginManager>& QEnginePluginManager::GetSingleton() {
	static QSharedPointer<QEnginePluginManager> Singleton(new QEnginePluginManager);
	return Singleton;
}

