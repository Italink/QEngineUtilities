#ifndef QDPI_Vector_h__
#define QDPI_Vector_h__

#include "QDetailWidgetPropertyItem.h"
#include "qvectornd.h"

class QDPI_Vector :public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(QVector2D,QVector3D,QVector4D);

	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_Vector_h__