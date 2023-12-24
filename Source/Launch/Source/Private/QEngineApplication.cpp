#include "QEngineApplication.h"
#include "Plugin/QEnginePluginManager.h"

QEngineApplication::QEngineApplication(int& argc, char** argv)
	: QApplication(argc,argv)
{
	QEnginePluginManager::Get().loadPlugins();
	for (auto& handler : QEnginePluginManager::Get().getPluginHandlers()) {
#ifndef QENGINE_WITH_EDITOR
		if (! handler.info.tags.contains("Editor"))
#endif
			handler.startup();
	}
}

QEngineApplication::~QEngineApplication() {
	for (auto& pluginInfo : QEnginePluginManager::Get().getPluginHandlers()) {
		pluginInfo.shutdown();
	}
	QEnginePluginManager::TearDown();
}

