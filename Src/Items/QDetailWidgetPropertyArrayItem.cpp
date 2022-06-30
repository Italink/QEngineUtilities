#include "QDetailWidgetPropertyArrayItem.h"
#include "QMetaType"
#include "QSequentialIterable"
#include "QDetailWidgetManager.h"
#include "QPushButton"

QDetailWidgetPropertyArrayItem::QDetailWidgetPropertyArrayItem()
{
	SetReorderChildrenEnabled(true);
}

bool QDetailWidgetPropertyArrayItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantList>())
		&& !QMetaType::canConvert(metaType, QMetaType::fromType<QString>())
		) {
		return true;
	}
	return false;
}

void QDetailWidgetPropertyArrayItem::SetHandler(QPropertyHandler* inHandler)
{
	QDetailWidgetPropertyItem::SetHandler(inHandler);
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	mValueTypeId = iterable.valueMetaType().id();
}

void QDetailWidgetPropertyArrayItem::ResetValue()
{
	QDetailWidgetPropertyItem::ResetValue();
	RecreateChildren();
}

void QDetailWidgetPropertyArrayItem::FindOrCreateChildItem(int index)
{
	if (index < childCount()) {
		QDetailWidgetPropertyItem* item = (QDetailWidgetPropertyItem*)child(index);
		item->SetHandler(QPropertyHandler::FindOrCreate(GetContent(),
			mValueTypeId,
			QString::number(index),
			[this, index]() {
				QVariant varList = GetValue();
				QSequentialIterable iterable = varList.value<QSequentialIterable>();
				QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
				return iterable.at(index);
			},
			[this, index](QVariant var) {
				QVariant varList = GetValue();
				QSequentialIterable iterable = varList.value<QSequentialIterable>();
				const QMetaSequence metaSequence = iterable.metaContainer();
				void* containterPtr = const_cast<void*>(iterable.constIterable());
				QtPrivate::QVariantTypeCoercer coercer;
				const void* dataPtr = coercer.coerce(var, var.metaType());
				metaSequence.setValueAtIndex(containterPtr, index, dataPtr);
				SetValue(varList);
			})
		);
	}
	else {
		QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(
			QPropertyHandler::FindOrCreate(GetContent(),
				mValueTypeId,
				QString::number(index),
				[this, index]() {
					QVariant varList = GetValue();
					QSequentialIterable iterable = varList.value<QSequentialIterable>();
					QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
					return iterable.at(index);
				},
				[this, index](QVariant var) {
					QVariant varList = GetValue();
					QSequentialIterable iterable = varList.value<QSequentialIterable>();
					const QMetaSequence metaSequence = iterable.metaContainer();
					void* containterPtr = const_cast<void*>(iterable.constIterable());
					QtPrivate::QVariantTypeCoercer coercer;
					const void* dataPtr = coercer.coerce(var, var.metaType());
					metaSequence.setValueAtIndex(containterPtr, index, dataPtr);
					SetValue(varList);
				}
					),
			GetMetaData());
		if (item) {
			item->AttachTo(this);
		}
	}
}

void QDetailWidgetPropertyArrayItem::RecreateChildren()
{
	QVariant var = GetValue();
	QSequentialIterable iterable = var.value<QSequentialIterable>();
	for (int i = 0; i < iterable.size(); i++) {
		FindOrCreateChildItem(i);
	}
	while (childCount()>iterable.size()){
		delete takeChild(iterable.size());
	}
}

void QDetailWidgetPropertyArrayItem::CreateNewItem()
{
	QVariant varList = GetValue();
	QSequentialIterable iterable = varList.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QMetaType valueType(mValueTypeId);
	QVariant var(valueType);
	const void* dataPtr = coercer.coerce(var, var.metaType());
	metaSequence.addValue(containterPtr,dataPtr);
	SetValue(varList);
	RecreateChildren();
}

QWidget* QDetailWidgetPropertyArrayItem::GenerateValueWidget() {
	QPushButton* btAppend = new QPushButton("+");
	connect(btAppend, &QPushButton::clicked, this, &QDetailWidgetPropertyArrayItem::CreateNewItem);
	return btAppend;
}

void QDetailWidgetPropertyArrayItem::BuildContentAndChildren() {
	GetContent()->SetNameWidgetByText(GetName());
	GetContent()->SetValueWidget(GenerateValueWidget());
	treeWidget()->setItemWidget(this, 0, GetContent());
	RecreateChildren();
}

