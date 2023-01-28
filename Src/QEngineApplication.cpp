#include "QEngineApplication.h"

QEngineApplication::QEngineApplication(int& argc, char** argv)
	: QApplication(argc,argv)
{
	qputenv("QSG_INFO", "1");
}

