#include "Plugin/QEnginePluginManager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>

typedef IEnginePlugin* (*InitializePluginFunc)();

QEnginePluginManager& QEnginePluginManager::Get() {
	return *GetSingleton().get();
}

void QEnginePluginManager::TearDown() {
	GetSingleton().reset();
}

void QEnginePluginManager::loadPlugin(QFileInfo fileInfo)
{
	QEnginePluginHandler info;
	info.handle.reset(new QLibrary(fileInfo.filePath()));
	if (info.handle->load()) {
		InitializePluginFunc InitializePlugin = (InitializePluginFunc)info.handle->resolve("InitializePlugin");
		if (InitializePlugin) {
			info.plugin.reset(InitializePlugin());
			mPluginMap[fileInfo.baseName()] = info;
			qDebug() << QString("-Load %1 Success").arg(info.handle->fileName()).toLatin1().constData();
			Q_EMIT asPluginChanged();
		}
		else {
			qDebug() << QString("-Load %1 Failed").arg(info.handle->fileName()).toLatin1().constData();
		}
	}

}

void QEnginePluginManager::loadPlugins() {
	//const QString PATH(QLatin1StringView(qgetenv("PATH")));
	//QStringList pathList = PATH.split(u';', Qt::SkipEmptyParts);
	//qDebug() << pathList;

	this->blockSignals(true);
	QDir pluginDir("./Plugins");
	for (const QFileInfo& fileInfo : pluginDir.entryInfoList(QDir::Files, QDir::IgnoreCase)) {
		if (fileInfo.suffix().compare("dll", Qt::CaseInsensitive) == 0) {
			loadPlugin(fileInfo);
		}
	}
	this->blockSignals(false);
	Q_EMIT asPluginChanged();
}

QMap<QString, QEnginePluginHandler>& QEnginePluginManager::getPluginMap()
{
	return mPluginMap;
}

QSharedPointer<QEnginePluginManager>& QEnginePluginManager::GetSingleton() {
	static QSharedPointer<QEnginePluginManager> Singleton(new QEnginePluginManager);
	return Singleton;
}

