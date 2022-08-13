#include "QDetailWidgetPropertyAssociativeItem.h"
#include "QMetaType"
#include "Customization\QDetailWidgetManager.h"
#include "QAssociativeIterable"
#include "QPushButton"

bool QDetailWidgetPropertyAssociativeItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

void QDetailWidgetPropertyAssociativeItem::SetHandler(QPropertyHandler* inHandler) {
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	QVariant var = GetValue();
	QAssociativeIterable iterable = var.value<QAssociativeIterable>();
	mValueTypeId = iterable.metaContainer().mappedMetaType().id();
}

void QDetailWidgetPropertyAssociativeItem::ResetValue() {
	QDetailWidgetPropertyItem::ResetValue();
	RecreateChildren();
}

QDetailWidgetPropertyItem* QDetailWidgetPropertyAssociativeItem::FindItem(QString inKey) {
	QDetailWidgetPropertyItem* item = nullptr;
	for (int i = 0; i < childCount(); i++) {
		QDetailWidgetPropertyItem* currItem = (QDetailWidgetPropertyItem*)child(i);
		if (currItem != nullptr && currItem->GetName() == inKey) {
			item = currItem;
			break;
		}
	}
	return item;
}

void QDetailWidgetPropertyAssociativeItem::FindOrCreateChildItem(QString inKey) {
	QDetailWidgetPropertyItem* item = FindItem(inKey);
	QPropertyHandler* handler = QPropertyHandler::FindOrCreate(
		GetParentObject(),
		mValueTypeId,
		GetHandler()->GetSubPath(inKey),
		[this, inKey]() {
		QVariant varMap = GetValue();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		return iterable.value(inKey);
	},
		[this, inKey](QVariant var) {
		QVariant varMap = GetValue();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		QMetaAssociation metaAssociation = iterable.metaContainer();

		QtPrivate::QVariantTypeCoercer keyCoercer;
		QtPrivate::QVariantTypeCoercer mappedCoercer;
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		const void* dataPtr = mappedCoercer.coerce(var, var.metaType());
		metaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(inKey, metaAssociation.keyMetaType()), dataPtr);
		SetValue(varMap);
	});
	if (item != nullptr) {
		item->SetHandler(handler);
	}
	else {
		QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(handler);
		if (item) {
			item->AttachTo(this);
			item->SetRenameCallback([this, item](QString name) {
				return RenameChild(item->GetName(), name);
			});
		}
	}
}

bool QDetailWidgetPropertyAssociativeItem::RenameChild(QString inSrc, QString inDst) {
	bool canRename = false;
	QVariant varMap = GetValue();
	QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
	if (iterable.containsKey(inSrc) && !iterable.containsKey(inDst)) {
		canRename = true;
		QVariant var = iterable.value(inSrc);
		QtPrivate::QVariantTypeCoercer keyCoercer;
		QtPrivate::QVariantTypeCoercer mappedCoercer;
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		QMetaAssociation metaAssociation = iterable.metaContainer();
		metaAssociation.removeKey(containterPtr, keyCoercer.coerce(inSrc, QMetaType::fromType<QString>()));
		//metaAssociation.insertKey(containterPtr, keyCoercer.coerce(inDst, QMetaType::fromType<QString>()));
		metaAssociation.setMappedAtKey(
			containterPtr,
			keyCoercer.coerce(inDst, QMetaType::fromType<QString>()),
			mappedCoercer.coerce(var, var.metaType())
		);;
		SetValue(varMap);
		RecreateChildren();
	}
	return canRename;
}

void QDetailWidgetPropertyAssociativeItem::RecreateChildren() {
	QVariant var = GetValue();
	QAssociativeIterable iterable = var.value<QAssociativeIterable>();
	for (auto iter = iterable.begin(); iter != iterable.end(); ++iter) {
		QString key = iter.key().toString();
		FindOrCreateChildItem(key);
	}
	int i = 0;
	while (i < childCount()) {
		QDetailWidgetPropertyItem* currItem = (QDetailWidgetPropertyItem*)child(i);
		if (currItem && !iterable.containsKey(currItem->GetName())) {
			delete takeChild(i);
		}
		else {
			i++;
		}
	}
}

void QDetailWidgetPropertyAssociativeItem::CreateNewItem() {
	QVariant varList = GetValue();
	QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
	const QMetaAssociation metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant var(QString("Item"));
	const void* dataPtr = coercer.coerce(var, var.metaType());
	metaSequence.insertKey(containterPtr, dataPtr);
	SetValue(varList);
	RecreateChildren();
	QDetailWidgetPropertyItem* item = FindItem("Item");
	if (item)
		item->RequestRename();
}

QWidget* QDetailWidgetPropertyAssociativeItem::GenerateValueWidget() {
	QPushButton* btAppend = new QPushButton("+");
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertyAssociativeItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertyAssociativeItem::BuildContentAndChildren() {
	GetContent()->SetNameWidgetByText(GetName());
	GetContent()->SetValueWidget(GenerateValueWidget());
	treeWidget()->setItemWidget(this, 0, GetContent());
	RecreateChildren();
}