#include "QDetailWidgetPropertyInstanceItem.h"
#include "QMetaType"
#include "QDetailWidgetManager.h"
#include "QAssociativeIterable"
#include "QPushButton"

bool QDetailWidgetPropertyInstanceItem::FilterType(TypeId inID) {
	return 	QMetaType::metaObjectForType(inID) != nullptr;
}

void QDetailWidgetPropertyInstanceItem::SetHandler(QPropertyHandler* inHandler)
{
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	mMetaObject = QMetaType::metaObjectForType(inHandler->GetTypeID());
}

void QDetailWidgetPropertyInstanceItem::ResetValue()
{
	QDetailWidgetPropertyItem::ResetValue();
	RecreateChildren();
}

void QDetailWidgetPropertyInstanceItem::RecreateChildren()
{

}

QWidget* QDetailWidgetPropertyInstanceItem::GenerateValueWidget() {
	return nullptr;
}

void QDetailWidgetPropertyInstanceItem::BuildContentAndChildren() {
	GetContent()->SetNameWidgetByText(GetName());
	GetContent()->SetValueWidget(GenerateValueWidget());
	treeWidget()->setItemWidget(this, 0, GetContent());
	RecreateChildren();
}