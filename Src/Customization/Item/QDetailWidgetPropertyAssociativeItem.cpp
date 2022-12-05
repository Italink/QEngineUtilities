#include "QDetailWidgetPropertyAssociativeItem.h"
#include "QMetaType"
#include "Customization\QDetailWidgetManager.h"
#include "QAssociativeIterable"
#include "Widgets\Toolkits\QSvgButton.h"

bool QDetailWidgetPropertyAssociativeItem::FilterType(QMetaType inType) {
	if (QMetaType::canConvert(inType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

void QDetailWidgetPropertyAssociativeItem::SetHandler(QPropertyHandler* inHandler) {
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	QVariant var = GetValue();
	QAssociativeIterable iterable = var.value<QAssociativeIterable>();
	mValueType = iterable.metaContainer().mappedMetaType();
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
		mValueType,
		GetHandler()->GetSubPath(inKey),
		[ParentHandle = GetHandler(), inKey]() {
		QVariant varMap = ParentHandle->GetValue();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		return iterable.value(inKey);
	},
		[ParentHandle = GetHandler(),inKey](QVariant var) {
		QVariant varMap = ParentHandle->GetValue();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		QMetaAssociation metaAssociation = iterable.metaContainer();

		QtPrivate::QVariantTypeCoercer keyCoercer;
		QtPrivate::QVariantTypeCoercer mappedCoercer;
		void* containterPtr = const_cast<void*>(iterable.constIterable());
		const void* dataPtr = mappedCoercer.coerce(var, var.metaType());
		metaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(inKey, metaAssociation.keyMetaType()), dataPtr);
		ParentHandle->SetValue(varMap);
	});
	if (item != nullptr) {
		item->SetHandler(handler);
	}
	else {
		QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(handler,GetInstance());
		if (item) {
			if (!GetHandler()->GetMetaData("FixedSize").toBool()) {
				item->SetBuildContentAndChildrenCallback([item, this, inKey](QHBoxLayout * inLayout) {
					QSvgButton* deleteButton = new QSvgButton(":/Resources/delete.png");
					deleteButton->setFixedSize(20, 20);
					inLayout->addWidget(deleteButton);
					connect(deleteButton, &QSvgButton::clicked, this, [inKey, this]() {
						RemoveItem(inKey);
					});
				});
			}
			if (!GetHandler()->GetMetaData("FixedKey").toBool()) {
				item->SetRenameCallback([this, item](QString name) {
					return RenameChild(item->GetName(), name);
				});
			}
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
	const QMetaAssociation metaAssociation = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QString newKey = "Item0";
	int index = 0;
	while (iterable.containsKey(newKey)) {
		newKey = "Item" + QString::number(++index);
	}
	QVariant key(newKey);
	QVariant value = QPropertyHandler::CreateNewVariant(mValueType);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	const void* valueDataPtr = coercer.coerce(value, value.metaType());
	//metaAssociation.insertKey(containterPtr, keyDataPtr);
	metaAssociation.setMappedAtKey(containterPtr, keyDataPtr, valueDataPtr);
	SetValue(varList,QString("%1 Insert: %2").arg(GetHandler()->GetPath()).arg(newKey));
	setExpanded(true);
	FindItem(newKey)->setExpanded(true);
}

void QDetailWidgetPropertyAssociativeItem::RemoveItem(QString inKey) {
	QVariant varList = GetValue();
	QAssociativeIterable iterable = varList.value<QAssociativeIterable>();
	const QMetaAssociation metaAssociation = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QVariant key(inKey);
	const void* keyDataPtr = coercer.coerce(key, key.metaType());
	metaAssociation.removeKey(containterPtr, keyDataPtr);
	SetValue(varList, QString("%1 Remove: %2").arg(GetHandler()->GetPath()).arg(inKey));
}

QWidget* QDetailWidgetPropertyAssociativeItem::GenerateValueWidget() {
	if (GetHandler()->GetMetaData("FixedSize").toBool()) {
		return nullptr;
	}
	QSvgButton* btAppend = new QSvgButton(":/Resources/plus.png");
	btAppend->setFixedSize(20, 20);
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertyAssociativeItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertyAssociativeItem::BuildContentAndChildren() {
	QDetailWidgetPropertyItem::BuildContentAndChildren();
	RecreateChildren();
}