#ifndef QDPI_Number_h__
#define QDPI_Number_h__

#include "QDetailWidgetPropertyItem.h"

#define Q_DETAIL_FOR_EACH_NUMBER_TYPE(FuncBegin,Func)\
	FuncBegin(int) \
	Func(float) \
	Func(double) \
	Func(short) \
	Func(unsigned int)

#define Q_DETAIL_NUMBER_PREPEND_COMMAN(Type) ,Type
#define Q_DETAIL_NUMBER_BEGIN(Type) Type

class QDPI_Number :public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(Q_DETAIL_FOR_EACH_NUMBER_TYPE(Q_DETAIL_NUMBER_BEGIN, Q_DETAIL_NUMBER_PREPEND_COMMAN));
	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_Number_h__