#ifndef CorePlugin_h__
#define CorePlugin_h__

#include "Plugin/IEnginePlugin.h"

class CorePlugin: public IEnginePlugin {
protected:
	void startup() override;
	void shutdown() override;
};

#endif // CorePlugin_h__
