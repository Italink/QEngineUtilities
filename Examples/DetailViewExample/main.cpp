#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "TestObject.h"
#include "qbasicatomic.h"
#include "DetailView/QDetailView.h"

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	//qRegisterMetaType<TestInlineObject>();
	//qRegisterMetaType<TestInlineObject*>();
	//qRegisterMetaType<TestInlineGadget>();
	//qRegisterMetaType<TestInlineGadget*>();
	//qRegisterMetaType<QSharedPointer<TestInlineGadget>>();
	//qRegisterMetaType<std::shared_ptr<TestInlineGadget>>();

	//TestInlineGadget gadget;
	TestObject obj;
	QDetailView widget;
	widget.setMinimumSize(200, 200);
	widget.setObject(&obj);
	widget.show();
	return app.exec();
}