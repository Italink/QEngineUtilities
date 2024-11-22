#ifndef QEnginePlugin_h__
#define QEnginePlugin_h__

#include <QUrl>
#include <QSet>
#include "QEngineCoreAPI.h"

class QENGINECORE_API IEnginePlugin {
public:
	struct Info
	{
		QUrl icon;
		QString name;
		QString author;
		QString link;
		QString description;
		QList<QUrl> images;
		QSet<QString> tags;
	};

	virtual ~IEnginePlugin(){}
	virtual Info info() = 0;
	virtual void startup() {}
	virtual void shutdown() {}
};

#define QENGINE_IMPLEMENT_PLUGIN(PluginClass, PluginName ) \
		extern "C" Q_DECL_EXPORT IEnginePlugin* InitializePlugin() \
		{ \
			return new PluginClass(); \
		} 

#endif // QEnginePlugin_h__
