#include "QApplication"
#include "qvariant.h"
#include "qsequentialiterable.h"
#include "TestObject.h"
#include "QDetailWidget.h"


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
	QDetailWidget widget(QDetailWidget::Flags(QDetailWidget::Flag::DisplayCategory|QDetailWidget::Flag::DisplaySearcher),QDetailWidget::Unreal);
	widget.SetInstances(&obj);
	widget.show();
	return app.exec();
}