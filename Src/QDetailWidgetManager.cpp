#include "QDetailWidgetManager.h"
#include "Items\Customization\QDPI_Number.h"
#include "Items\QDetailWidgetPropertyArrayItem.h"
#include "Items\Customization\QDPI_Bool.h"
#include "Items\Customization\QDPI_Color.h"
#include "Items\Customization\QDPI_Colors.h"
#include "Items\Customization\QDPI_String.h"
#include "Items\Customization\QDPI_Vector.h"


QDetailWidgetManager::QDetailWidgetManager()
{
	RegisterBuiltIn();
}

QDetailWidgetManager* QDetailWidgetManager::instance()
{
	static QDetailWidgetManager ins;
	return &ins;
}

QDetailWidgetPropertyItem* QDetailWidgetManager::CreatePropertyItem(QDetailWidgetPropertyItem::TypeId inTypeID, QString inName, QDetailWidgetPropertyItem::Getter inGetter, QDetailWidgetPropertyItem::Setter inSetter, QJsonObject inMetaData) {
	for (auto& filter : mPropertyItemFilterList) {
		if (filter.first(inTypeID)) {
			QDetailWidgetPropertyItem* item = filter.second();
			item->Initialize(inTypeID, inName, inGetter, inSetter, inMetaData);
			return item;
		}
	}

	auto iter = mPropertyItemCreatorMap.find(inTypeID);
	if (iter != mPropertyItemCreatorMap.end()) {
		QDetailWidgetPropertyItem* item = (*iter)();
		item->Initialize(inTypeID, inName, inGetter, inSetter,inMetaData);
		return item;
	}

	qWarning() << QString("Name: %1, TypeID: %2 : is not registered !").arg(inName).arg(inTypeID);
	return nullptr;
}

void QDetailWidgetManager::RegisterBuiltIn()
{
	RegisterPropertyItemCreator<QDPI_Bool>();
	RegisterPropertyItemCreator<QDPI_Color>();
	RegisterPropertyItemCreator<QDPI_Colors>();
	RegisterPropertyItemCreator<QDPI_Number>();
	RegisterPropertyItemCreator<QDPI_String>();
	RegisterPropertyItemCreator<QDPI_Vector>();
	RegisterPropertyItemFilter<QDetailWidgetPropertyArrayItem>();
	//RegisterProperty<int, QPC_Number<int>>();
	//RegisterProperty<float, QPC_Number<float>>();
	//RegisterProperty<double, QPC_Number<double>>();
	//RegisterProperty<bool, QPC_Bool>();
	//RegisterProperty<QString, QPC_String>();
	//RegisterProperty<std::string, QPC_String>();
	//RegisterProperty<QVector2D, QPC_Vector>();
	//RegisterProperty<QVector3D, QPC_Vector>();
	//RegisterProperty<QVector4D, QPC_Vector>();
	//RegisterProperty<QColor, QPC_Color>();
	//RegisterProperty<QColors, QPC_Colors>();
}
