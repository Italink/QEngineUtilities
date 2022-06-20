#ifndef QDPI_Colors_h__
#define QDPI_Colors_h__

#include "Items\QDetailWidgetPropertyItem.h"
#include "Types\QColors.h"

class QDPI_Colors :public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(QColors);
	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_Colors_h__