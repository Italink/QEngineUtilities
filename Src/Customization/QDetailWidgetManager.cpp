#include "QDetailWidgetManager.h"

#include "Item\QDPI_Bool.h"
#include "Item\QDPI_Color.h"
#include "Item\QDPI_Colors.h"
#include "Item\QDPI_Number.h"
#include "Item\QDPI_String.h"
#include "Item\QDPI_Vector.h"
#include "Item\QDetailWidgetPropertySequentialItem.h"
#include "Item\QDetailWidgetPropertyEnumItem.h"
#include "Item\QDetailWidgetPropertyAssociativeItem.h"
#include "Item\QDetailWidgetPropertyInstanceItem.h"
#include "Instance\QInstanceDetail_QObject.h"

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
	RegisterPropertyItemFilter<QDetailWidgetPropertyInstanceItem>();

	RegisterInstanceFilter<QInstanceDetail_QObject>();
}

