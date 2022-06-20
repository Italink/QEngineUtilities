#ifndef QDPI_Color_h__
#define QDPI_Color_h__

#include "Items\QDetailWidgetPropertyItem.h"

class QDPI_Color :public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(QColor);
	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_Color_h__