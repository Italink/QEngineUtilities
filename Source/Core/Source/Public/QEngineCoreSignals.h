#ifndef QEngineCoreSignals_h__
#define QEngineCoreSignals_h__

#include <QObject>
#include <QKeyEvent>
#include "QEngineCoreAPI.h"

class QENGINECORE_API QEngineCoreSignals: public QObject {
	Q_OBJECT
public:
	static QEngineCoreSignals* Instance();
Q_SIGNALS:
	void asStartRendering();
	void asStopRendering();
	void asViewportKeyPressEvent(QKeyEvent* event);
};

#endif // QEngineCoreSignals_h__
