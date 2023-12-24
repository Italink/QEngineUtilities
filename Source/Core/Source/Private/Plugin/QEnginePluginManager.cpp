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
	QEnginePluginHandler handler;
	handler.handle.reset(new QLibrary(fileInfo.filePath()));
	if (handler.handle->load()) {
		InitializePluginFunc InitializePlugin = (InitializePluginFunc)handler.handle->resolve("InitializePlugin");
		if (InitializePlugin) {
			handler.plugin.reset(InitializePlugin());
			handler.info = handler.plugin->info();
			mPluginHandlers << handler;
			qDebug() << QString("-Load %1 Success").arg(handler.handle->fileName()).toLatin1().constData();
			Q_EMIT asPluginChanged();
		}
		else {
			qDebug() << QString("-Load %1 Failed").arg(handler.handle->fileName()).toLatin1().constData();
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

QList<QEnginePluginHandler>& QEnginePluginManager::getPluginHandlers()
{
	return mPluginHandlers;
}

QSharedPointer<QEnginePluginManager>& QEnginePluginManager::GetSingleton() {
	static QSharedPointer<QEnginePluginManager> Singleton(new QEnginePluginManager);
	return Singleton;
}

