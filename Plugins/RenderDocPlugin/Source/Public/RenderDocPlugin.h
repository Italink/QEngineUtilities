#ifndef RenderDocPlugin_h__
#define RenderDocPlugin_h__

#include "Plugin/IEnginePlugin.h"
#include "QLibrary"

class RenderDocPlugin: public IEnginePlugin {
protected:
	void startup() override;
	void shutdown() override;
private:
	QLibrary renderdoc{ "renderdoc.dll" };
};

#endif // RenderDocPlugin_h__