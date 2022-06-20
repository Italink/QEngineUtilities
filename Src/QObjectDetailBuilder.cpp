#include "QObjectDetailBuilder.h"
#include "QDetailWidget.h"
#include "QJsonObject"
#include "qmetaobject.h"
#include "QMetaProperty"
#include "QMetaType"
#include "QSequentialIterable"
#include "QDetailWidgetManager.h"
#include "Items\QDetailWidgetCategoryItem.h"

QObjectDetailBuilder::QObjectDetailBuilder(QObject* mObject, QDetailWidget* inWidget)
	: mObject(mObject)
	, mWidget(inWidget)
{
	ReadObjectMetaData();
}

void QObjectDetailBuilder::BuildDefault()
{
	for (int i = 1; i < mObject->metaObject()->propertyCount(); i++) {
		QMetaProperty prop = mObject->metaObject()->property(i);
		if (!prop.isDesignable())
			continue;
		AddNewProperty(prop);
	}
}

void QObjectDetailBuilder::AddNewProperty(QMetaProperty inProperty)
{
	QDetailWidgetPropertyItem* item = QDetailWidgetManager::instance()->CreatePropertyItem(
		inProperty.typeId(),
		inProperty.name(),
		[Object = mObject, inProperty]() {return inProperty.read(Object); },
		[Object = mObject, inProperty](QVariant var) { inProperty.write(Object, var); },
		GetPropertyMetaData(inProperty.name())
	);

	QDetailWidgetCategoryItem* categoryItem = mWidget->FindOrAddCategory(GetPropertyCategoryName(inProperty.name()));
	if (item) {
		item->AttachTo(categoryItem);
	}



	//if (var.canConvert<QVariantList>()&&!var.canConvert<QString>()) {
		//QSequentialIterable iterable = var.value<QSequentialIterable>();
		/*	QDetailWidgetRow* listHeader = AddNewRow(GetPropertyCategoryName(inProperty.name()));
			listHeader->SetNameWidgetByText(inProperty.name());
			for (int i = 0; i < iterable.size(); i++) {
				QDetailWidgetRow* elementRow = new QDetailWidgetRow;
				listHeader->addChild(elementRow);

				QPropertyHandle* propHandle = new QPropertyHandle(
					mWidget,
					iterable.valueMetaType(),
					QString::number(i),
					[Object = mObject, inProperty,i]() {
						QVariant varList = inProperty.read(Object);
						QSequentialIterable iterable = varList.value<QSequentialIterable>();
						QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
						return iterable.at(i);
					},
					[Object = mObject, inProperty, i](QVariant var) {
						QVariant varList = inProperty.read(Object);
						QSequentialIterable iterable = varList.value<QSequentialIterable>();
						const QMetaSequence metaSequence = iterable.metaContainer();
						void* containterPtr = const_cast<void*>(iterable.constIterable());
						QtPrivate::QVariantTypeCoercer coercer;
						const void* dataPtr = coercer.coerce(var, var.metaType());
						metaSequence.setValueAtIndex(containterPtr, i, dataPtr);
						inProperty.write(Object, varList);
					}
					);
				QSharedPointer<QPropertyBuilder> propertyBuilder = QSharedPointer<QPropertyBuilder>::create(propHandle, elementRow, GetPropertyMetaData(inProperty.name()));
				propertyBuilder->BuildDefault();*/
		//}
	//}
	//else if (var.canConvert<QVariantHash>()) {

	//}
	//else if (var.canConvert<QVariantMap>()) {

	//}
	//else {
	//}
}


QString QObjectDetailBuilder::GetPropertyCategoryName(QString inPropertyName)
{
	QString categoryName = mMetaData.mPropertyMap.value(inPropertyName)["Category"].toString();
	if (categoryName.isEmpty())
		categoryName = mObject->objectName();
	if (categoryName.isEmpty())
		categoryName = "Common";
	return categoryName;
}

QJsonObject QObjectDetailBuilder::GetPropertyMetaData(QString inPropertyName)
{
	return mMetaData.mPropertyMap.value(inPropertyName);
}

void QObjectDetailBuilder::ReadObjectMetaData()
{
	for (int i = 0; i < mObject->metaObject()->methodCount(); i++) {
		QMetaMethod method = mObject->metaObject()->method(i);
		if (QString(method.name()).endsWith("_GetMetaData")) {
			QObjectMetaData MetaData;
			if (method.invoke(mObject, Q_RETURN_ARG(QObjectMetaData, MetaData))) {
				for (auto PropertyIter = MetaData.mPropertyMap.begin(); PropertyIter != MetaData.mPropertyMap.end(); ++PropertyIter) {
					 mMetaData.mPropertyMap[PropertyIter.key()] = PropertyIter.value();
				}
				for (auto categoryName : MetaData.mCategoryList) {
					mMetaData.mCategoryList << categoryName;
				}
			}
		}
	}
	for (auto categoryName : mMetaData.mCategoryList) {
		mWidget->FindOrAddCategory(categoryName);
	}
}
