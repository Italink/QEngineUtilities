#include "QDetailWidgetPropertyInstanceItem.h"
#include "Core\QInstance.h"
#include "Customization\QDetailWidgetManager.h"
#include "QAssociativeIterable"
#include "QMetaType"

bool QDetailWidgetPropertyInstanceItem::FilterType(TypeId inID) {
	return 	QMetaType::metaObjectForType(inID) != nullptr;
}

void QDetailWidgetPropertyInstanceItem::SetHandler(QPropertyHandler* inHandler)
{
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	mMetaObject = QMetaType::metaObjectForType(inHandler->GetTypeID());
	RecreateInstance();
}

void QDetailWidgetPropertyInstanceItem::ResetValue()
{
	QDetailWidgetPropertyItem::ResetValue();
	RecreateInstance();
	RecreateChildren();
}

void QDetailWidgetPropertyInstanceItem::RecreateInstance() {
	mInstanceVar = GetValue();
	if (mMetaObject->inherits(&QObject::staticMetaObject)) {
		QObject* object = mInstanceVar.value<QObject*>();
		mInstance = QInstance::CreateObjcet(object);
	}
	else {
		mInstance = QInstance::CreateGadget(mInstanceVar.data(), mMetaObject);
		mInstance->SetPropertyChangedCallback([this]() {
			GetHandler()->SetValue(mInstanceVar);
		});
	}
}

void QDetailWidgetPropertyInstanceItem::RecreateChildren()
{
	while (childCount() > 0) {
		delete takeChild(0);
	}
	if (mInstance->IsValid()) {
		QInstanceDetail* detail = QInstanceDetail::FindOrCreate(mInstance, this);
		detail->Build();
	}
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