#include "QDetailWidgetPropertyMapItem.h"
#include "QMetaType"
#include "QDetailWidgetManager.h"
#include "QAssociativeIterable"
#include "QPushButton"

bool QDetailWidgetPropertyMapItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

void QDetailWidgetPropertyMapItem::SetHandler(QPropertyHandler* inHandler)
{
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	QVariant var = GetValue();
	QAssociativeIterable iterable = var.value<QAssociativeIterable>();
	mValueTypeId = iterable.metaContainer().mappedMetaType().id();
}

void QDetailWidgetPropertyMapItem::ResetValue()
{
	QDetailWidgetPropertyItem::ResetValue();
	RecreateChildren();
}

QDetailWidgetPropertyItem* QDetailWidgetPropertyMapItem::FindItem(QString inKey)
{
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

void QDetailWidgetPropertyMapItem::FindOrCreateChildItem(QString inKey)
{
	QDetailWidgetPropertyItem* item = FindItem(inKey);
	if (item != nullptr) {
		item->SetHandler(QPropertyHandler::FindOrCreate(
			GetContent(),
			mValueTypeId,
			inKey,
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
			})
		);
	}
	else {
		QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(
			QPropertyHandler::FindOrCreate(
				GetContent(),
				mValueTypeId,
				inKey,
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
				}),
			GetMetaData()
					);
		if (item) {
			item->AttachTo(this);
			item->SetRenameCallback([this,item](QString name) {
				return RenameChild(item->GetName(), name);
			});
		}
	}
}

bool QDetailWidgetPropertyMapItem::RenameChild(QString inSrc, QString inDst)
{
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
	}
	return canRename;
}

void QDetailWidgetPropertyMapItem::RecreateChildren()
{
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

void QDetailWidgetPropertyMapItem::CreateNewItem()
{
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

QWidget* QDetailWidgetPropertyMapItem::GenerateValueWidget() {
	QPushButton* btAppend = new QPushButton("+");
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertyMapItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertyMapItem::BuildContentAndChildren() {
	GetContent()->SetNameWidgetByText(GetName());
	GetContent()->SetValueWidget(GenerateValueWidget());
	treeWidget()->setItemWidget(this, 0, GetContent());
	RecreateChildren();
}

