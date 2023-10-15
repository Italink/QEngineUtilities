#ifndef QEngineApplication_h__
#define QEngineApplication_h__

#include <QApplication>
#include "QEngineLaunchAPI.h"

class QENGINELAUNCH_API QEngineApplication : public QApplication {
public:
	QEngineApplication(int& argc, char** argv);
	virtual ~QEngineApplication();
};

#endif // QEngineApplication_h__
