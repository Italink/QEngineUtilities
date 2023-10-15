#include "QEngineApplication.h"
#include "Plugin/QEnginePluginManager.h"

QEngineApplication::QEngineApplication(int& argc, char** argv)
	: QApplication(argc,argv)
{
	//QEnginePluginManager::Get().loadConfig(QENGINE_PLUGINS_JSON_FILE);
	QEnginePluginManager::Get().startupPlugins("Runtime");
#ifdef QENGINE_WITH_EDITOR
	QEnginePluginManager::Get().startupPlugins("Editor");
#endif
}

QEngineApplication::~QEngineApplication() {
	QEnginePluginManager::Get().shutdownPlugins("Runtime");
#ifdef QENGINE_WITH_EDITOR
	QEnginePluginManager::Get().shutdownPlugins("Editor");
#endif
}

