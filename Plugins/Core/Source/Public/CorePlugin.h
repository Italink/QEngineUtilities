#ifndef CorePlugin_h__
#define CorePlugin_h__

#include "Plugin/IEnginePlugin.h"

class CorePlugin: public IEnginePlugin {
protected:	
	Type type() override { return IEnginePlugin::Core; }
	void startup() override;
	void shutdown() override;
};

#endif // CorePlugin_h__
