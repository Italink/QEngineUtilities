#ifndef QEnginePlugin_h__
#define QEnginePlugin_h__

#include "QEngineCoreAPI.h"

class QENGINECORE_API IEnginePlugin {
public:
	virtual ~IEnginePlugin(){}
	virtual void startup() {}
	virtual void shutdown() {}
};

#define QENGINE_IMPLEMENT_PLUGIN(PluginClass, PluginName ) \
		extern "C" Q_DECL_EXPORT IEnginePlugin* InitializePlugin() \
		{ \
			return new PluginClass(); \
		} 

#endif // QEnginePlugin_h__
