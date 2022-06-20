#ifndef QDPI_Bool_h__
#define QDPI_Bool_h__

#include "Items\QDetailWidgetPropertyItem.h"

class QDPI_Bool :public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(bool);
	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_Bool_h__