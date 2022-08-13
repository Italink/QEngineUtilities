#include "QObjectDetailBuilder.h"
#include "QDetailWidgetPrivate.h"
#include "QJsonObject"
#include "qmetaobject.h"
#include "QMetaProperty"
#include "QMetaType"
#include "QSequentialIterable"
#include "QDetailWidgetManager.h"
#include "Items\QDetailWidgetCategoryItem.h"

QObjectDetailBuilder::QObjectDetailBuilder(QSharedPointer<QInstance> inInstance, QDetailTreeWidget* inWidget)
	: mInstance(inInstance)
	, mWidget(inWidget)
{
	ReadObjectMetaData();
}

void QObjectDetailBuilder::BuildDefault()
{
	for (int i = 0; i < mInstance->GetMetaObject()->propertyCount(); i++) {
		QMetaProperty prop = mInstance->GetMetaObject()->property(i);
		if (!prop.isDesignable())
			continue;
		AddNewProperty(prop);
	}
}

void QObjectDetailBuilder::AddNewProperty(QMetaProperty inProperty)
{
	QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(
		QPropertyHandler::FindOrCreate(
			mInstance->GetOuterObject(),
			inProperty.typeId(),
			inProperty.name(),
			[Instance = mInstance.get(), inProperty]() {return Instance->GetProperty(inProperty); },
			[Instance = mInstance.get(), inProperty](QVariant var) { Instance->SetProperty(inProperty, var); }
		),
		GetPropertyMetaData(inProperty)
	);
	if (item) {
		QDetailWidgetCategoryItem* categoryItem = mWidget->FindOrAddCategory(GetPropertyCategoryName(inProperty.name()));
		item->AttachTo(categoryItem);
	}
	else {
		auto it = QMetaType::metaObjectForType(inProperty.typeId());
		//auto it = MetaObjectForType
		qDebug() << inProperty.name();
	}
}

QString QObjectDetailBuilder::GetPropertyCategoryName(QString inPropertyName)
{
	QString categoryName = mMetaData.mPropertiesMetaData.value(inPropertyName)["Category"].toString();
	if (categoryName.isEmpty())
		categoryName = mInstance->GetOuterObject()->objectName();
	if (categoryName.isEmpty())
		categoryName = "Common";
	return categoryName;
}

QVariantHash QObjectDetailBuilder::GetPropertyMetaData(QMetaProperty inProperty)
{
	return mMetaData.mPropertiesMetaData.value(inProperty.name());
}

void QObjectDetailBuilder::ReadObjectMetaData()
{
	for (int i = 0; i < mInstance->GetMetaObject()->methodCount(); i++) {
		QMetaMethod method = mInstance->GetMetaObject()->method(i);
		if (QString(method.name()).endsWith("_GetMetaData")) {
			QObjectMetaData MetaData;
			if (mInstance->Invoke(method, Q_RETURN_ARG(QObjectMetaData, MetaData))) {
				for (auto PropertyIter = MetaData.mPropertiesMetaData.begin(); PropertyIter != MetaData.mPropertiesMetaData.end(); ++PropertyIter) {
					 mMetaData.mPropertiesMetaData[PropertyIter.key()] = PropertyIter.value();
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
