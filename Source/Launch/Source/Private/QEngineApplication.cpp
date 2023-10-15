#include "QEngineApplication.h"
#include "Plugin/QEnginePluginManager.h"

QEngineApplication::QEngineApplication(int& argc, char** argv)
	: QApplication(argc,argv)
{
	QEnginePluginManager::Get().loadPlugins();
	for (auto& pluginInfo : QEnginePluginManager::Get().getPluginMap()) {
#ifndef QENGINE_WITH_EDITOR
		if (pluginInfo.plugin->type() == IEnginePlugin::Core)
#endif
		pluginInfo.startup();
	}
}

QEngineApplication::~QEngineApplication() {
	for (auto& pluginInfo : QEnginePluginManager::Get().getPluginMap()) {
		pluginInfo.shutdown();
	}
	QEnginePluginManager::TearDown();
}

