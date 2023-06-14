#ifndef QEngineCoreEntry_h__
#define QEngineCoreEntry_h__

#include <QObject>
#include <QKeyEvent>
#include "QEngineCoreAPI.h"

class QENGINECORE_API QEngineCoreEntry: public QObject {
	Q_OBJECT
public:
	static QEngineCoreEntry* Instance();
Q_SIGNALS:
	void asStartRendering();
	void asStopRendering();
	void asViewportKeyPressEvent(QKeyEvent* event);
};

#endif // QEngineCoreEntry_h__
