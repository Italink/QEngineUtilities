#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "TestObject.h"
#include "QDetailWidget.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	TestInlineGadget gadget;
	TestObject obj;

	QDetailWidget widget;
	widget.SetInstances(&obj, &gadget);
	widget.show();

	QDetailWidget gadgetwidget;
	gadgetwidget.SetInstances(&gadget);
	gadgetwidget.show();

	return app.exec();
}