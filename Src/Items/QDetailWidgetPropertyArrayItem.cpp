#include "QDetailWidgetPropertyArrayItem.h"
#include "QMetaType"
#include "QSequentialIterable"
#include "QDetailWidgetManager.h"

bool QDetailWidgetPropertyArrayItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantList>())
		&& !QMetaType::canConvert(metaType, QMetaType::fromType<QString>())
		) {
		return true;
	}
	return false;
}

QWidget* QDetailWidgetPropertyArrayItem::GenerateValueWidget() {
	return nullptr;
}

void QDetailWidgetPropertyArrayItem::BuildContentAndChildren() {
	GetContent()->SetNameWidgetByText(GetName());
	treeWidget()->setItemWidget(this, 0, GetContent());
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	for (int i = 0; i < iterable.size(); i++) {
		QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(
			QPropertyHandler::FindOrCreate(GetContent(),
				iterable.valueMetaType().id(),
				QString::number(i),
				[this, i]() {
					QVariant varList = GetValue();
					QSequentialIterable iterable = varList.value<QSequentialIterable>();
					QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
					return iterable.at(i);
				},
				[this, i](QVariant var) {
					QVariant varList = GetValue();
					QSequentialIterable iterable = varList.value<QSequentialIterable>();
					const QMetaSequence metaSequence = iterable.metaContainer();
					void* containterPtr = const_cast<void*>(iterable.constIterable());
					QtPrivate::QVariantTypeCoercer coercer;
					const void* dataPtr = coercer.coerce(var, var.metaType());
					metaSequence.setValueAtIndex(containterPtr, i, dataPtr);
					SetValue(varList);
				}
				), 
			GetMetaData());
		if(item){
			item->AttachTo(this);
		}
	}

}

