#ifndef RenderDocPlugin_h__
#define RenderDocPlugin_h__

#include "Plugin/IEnginePlugin.h"
#include "QLibrary"

class RenderDocPlugin: public IEnginePlugin {
protected:
	Type type() override { return IEnginePlugin::Editor; }
	void startup() override;
	void shutdown() override;
private:
	QLibrary renderdoc{ "C:/Program Files/RenderDoc/renderdoc.dll" };
};

#endif // RenderDocPlugin_h__