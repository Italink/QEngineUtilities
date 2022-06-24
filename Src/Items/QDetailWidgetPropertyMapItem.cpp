#include "QDetailWidgetPropertyMapItem.h"
#include "QMetaType"
#include "QDetailWidgetManager.h"
#include "QAssociativeIterable"

bool QDetailWidgetPropertyMapItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

QWidget* QDetailWidgetPropertyMapItem::GenerateValueWidget() {
	return nullptr;
}

void QDetailWidgetPropertyMapItem::BuildContentAndChildren() {
	GetContent()->SetNameWidgetByText(GetName());
	treeWidget()->setItemWidget(this, 0, GetContent());
	QVariant var = GetValue();

	QAssociativeIterable iterable = var.value<QAssociativeIterable>();

	for (auto iter = iterable.begin(); iter != iterable.end(); ++iter) {
		QString key = iter.key().toString();
		QDetailWidgetPropertyItem* item = QDetailWidgetManager::instance()->CreatePropertyItem(
			iter->metaType().id(),
			key,
			[this, key]() {
				QVariant varMap = GetValue();
				QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
				return iterable.value(key);
			},
			[this, key](QVariant var) {
				QVariant varMap = GetValue();
				QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
				QMetaAssociation metaAssociation = iterable.metaContainer();

				QtPrivate::QVariantTypeCoercer keyCoercer;
				QtPrivate::QVariantTypeCoercer mappedCoercer;
				void* containterPtr = const_cast<void*>(iterable.constIterable());
				const void* dataPtr = mappedCoercer.coerce(var, var.metaType());
				metaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(key, metaAssociation.keyMetaType()),	dataPtr);
				SetValue(varMap);
			},
				GetMetaData()
				);
		if (item) {
			item->AttachTo(this);
		}
	}
}

