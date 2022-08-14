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
	connect(GetHandler(), &QPropertyHandler::AsValueChanged, this, &QDetailWidgetPropertyAssociativeItem::RecreateChildren);
}

void QDetailWidgetPropertyAssociativeItem::ResetValue() {
	QDetailWidgetPropertyItem::ResetValue();
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
			item->SetBuildContentAndChildrenCallback([item]() {
				item->AddValueWidget(new QPushButton("HaHa"));
			});
			item->SetRenameCallback([this, item](QString name) {
				return RenameChild(item->GetName(), name);
			});
			item->AttachTo(this);
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
		metaAssociation.setMappedAtKey(
			containterPtr,
			keyCoercer.coerce(inDst, QMetaType::fromType<QString>()),
			mappedCoercer.coerce(var, var.metaType())
		);;
		SetValue(varMap, QString("Rename: %1 -> %2").arg(inSrc).arg(inDst));
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
	QString newKey = "Item0";
	int index = 0;
	while (iterable.containsKey(newKey)) {
		newKey = "Item" + QString::number(++index);
	}
	QVariant key(newKey);
	QVariant value = QPropertyHandler::CreateNewVariant(mValueTypeId);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	const void* valueDataPtr = coercer.coerce(value, value.metaType());
	//metaSequence.insertKey(containterPtr, keyDataPtr);
	metaSequence.setMappedAtKey(containterPtr, keyDataPtr, valueDataPtr);
	SetValue(varList,QString("%1 Insert: %2").arg(GetHandler()->GetPath()).arg(newKey));
	setExpanded(true);
	FindItem(newKey)->setExpanded(true);
}

QWidget* QDetailWidgetPropertyAssociativeItem::GenerateValueWidget() {
	QPushButton* btAppend = new QPushButton("+");
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertyAssociativeItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertyAssociativeItem::BuildContentAndChildren() {
	QDetailWidgetPropertyItem::BuildContentAndChildren();
	RecreateChildren();
}