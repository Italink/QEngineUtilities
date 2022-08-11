#include "QDetailWidgetManager.h"
#include "Items\Customization\QDPI_Bool.h"
#include "Items\Customization\QDPI_Color.h"
#include "Items\Customization\QDPI_Colors.h"
#include "Items\Customization\QDPI_Number.h"
#include "Items\Customization\QDPI_String.h"
#include "Items\Customization\QDPI_Vector.h"
#include "Items\QDetailWidgetPropertySequentialItem.h"
#include "Items\QDetailWidgetPropertyEnumItem.h"
#include "Items\QDetailWidgetPropertyAssociativeItem.h"


QDetailWidgetManager::QDetailWidgetManager()
{
	Q_INIT_RESOURCE(Resources);
	RegisterBuiltIn();
}

QDetailWidgetManager* QDetailWidgetManager::instance()
{
	static QDetailWidgetManager ins;
	return &ins;
}

void QDetailWidgetManager::RegisterBuiltIn()
{
	RegisterPropertyItemCreator<QDPI_Bool>();
	RegisterPropertyItemCreator<QDPI_Color>();
	//RegisterPropertyItemCreator<QDPI_Colors>();
	RegisterPropertyItemCreator<QDPI_Number>();
	RegisterPropertyItemCreator<QDPI_String>();
	RegisterPropertyItemCreator<QDPI_Vector>();

	RegisterPropertyItemFilter<QDetailWidgetPropertyEnumItem>();
	RegisterPropertyItemFilter<QDetailWidgetPropertySequentialItem>();
	RegisterPropertyItemFilter<QDetailWidgetPropertyAssociativeItem>();
}
