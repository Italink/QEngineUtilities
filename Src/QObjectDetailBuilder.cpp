#include "QObjectDetailBuilder.h"
#include "QDetailWidgetPrivate.h"
#include "QJsonObject"
#include "qmetaobject.h"
#include "QMetaProperty"
#include "QMetaType"
#include "QSequentialIterable"
#include "QDetailWidgetManager.h"
#include "Items\QDetailWidgetCategoryItem.h"

QObjectDetailBuilder::QObjectDetailBuilder(QObject* mObject, QDetailTreeWidget* inWidget)
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
	QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(
		QPropertyHandler::FindOrCreate(mObject,
			inProperty.typeId(),
			inProperty.name(),
			[Object = mObject, inProperty]() {return inProperty.read(Object); },
			[Object = mObject, inProperty](QVariant var) { inProperty.write(Object, var); }
		),
		GetPropertyMetaData(inProperty)
	);

	QDetailWidgetCategoryItem* categoryItem = mWidget->FindOrAddCategory(GetPropertyCategoryName(inProperty.name()));
	if (item) {
		item->AttachTo(categoryItem);
	}
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


QJsonObject QObjectDetailBuilder::GetPropertyMetaData(QMetaProperty inProperty)
{
	if (inProperty.isEnumType()) {
		QMetaEnum Enum = inProperty.enumerator();
		QJsonObject metaData;
		QJsonArray enumList;
		for (int i = 0; i < Enum.keyCount(); i++) {
			QJsonObject enumData;
			enumData["Name"] = Enum.key(i);
			enumData["Value"] = Enum.value(i);
			enumList << enumData;
		}
		metaData["EnumList"] = enumList;
		return metaData;
	}
	return mMetaData.mPropertyMap.value(inProperty.name());
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
