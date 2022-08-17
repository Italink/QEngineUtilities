#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "TestObject.h"
#include "QDetailWidget.h"
#include "qbasicatomic.h"


int main(int argc, char** argv) {
	QApplication app(argc, argv);

	qRegisterMetaType<TestInlineObject>();
	qRegisterMetaType<TestInlineObject*>();
	qRegisterMetaType<TestInlineGadget>();
	qRegisterMetaType<TestInlineGadget*>();
	qRegisterMetaType<QSharedPointer<TestInlineGadget>>();
	qRegisterMetaType<std::shared_ptr<TestInlineGadget>>();

	TestInlineGadget gadget;
	TestObject obj;
	QDetailWidget widget(QDetailWidgetFlag::DisplaySearcher);
	widget.SetInstances(&obj);
	widget.show();

	return app.exec();
}