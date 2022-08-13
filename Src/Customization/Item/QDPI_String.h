#ifndef QDPI_String_h__
#define QDPI_String_h__

#include <string>
#include "QDetailWidgetPropertyItem.h"
#include "QString"

Q_DECLARE_METATYPE(std::string);

class QDPI_String :public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(QString,std::string);
	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_String_h__