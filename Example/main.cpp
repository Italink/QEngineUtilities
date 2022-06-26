#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "TestObject.h"
#include "QDetailWidget.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	QDetailWidget widget;
	TestObject obj;
	widget.SetObjects({&obj});
	widget.show();

	return app.exec();
}