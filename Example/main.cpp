#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "QDetailWidget.h"
#include "TestObject.h"
#include "qobjectdefs.h"
#include "qlogging.h"
#include "QMetaType"

#include <iostream>



int main(int argc, char** argv) {
	QApplication app(argc, argv);
	QDetailWidget widget;
	TestObject obj;
	widget.SetObjects({&obj});
	widget.show();


	return app.exec();

}