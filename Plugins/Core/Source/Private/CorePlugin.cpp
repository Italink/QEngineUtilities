#include "../Public/CorePlugin.h"
#include <QDebug>

void CorePlugin::startup() {
	qDebug() << "CorePlugin::startup";
}

void CorePlugin::shutdown() {
	qDebug() << "CorePlugin::shutdown";
}

QENGINE_IMPLEMENT_PLUGIN(CorePlugin,Core)