#include "QDPI_Number.h"
#include "Widgets\QNumberBox.h"

#define Q_DETAIL_NUMBER_SWITCH_CASE(Type) \
	case QMetaTypeId2<Type>::qt_metatype_id(): {	\
		if (MetaData.contains("Min") && MetaData.contains("Max")) { \
			QNumberBox* numberBox = new QNumberBox( \
				GetValue().value<Type>(), \
				true, \
				MetaData["Min"].toVariant().value<Type>(), \
				MetaData["Max"].toVariant().value<Type>() \
			); \
			connect(numberBox, &QNumberBox::AsValueChanged, this, &QDPI_Number::SetValue);\
			return numberBox; \
		} \
		else { \
			QNumberBox* numberBox = new QNumberBox(GetValue().value<Type>()); \
			connect(numberBox, &QNumberBox::AsValueChanged, this, &QDPI_Number::SetValue); \
			return numberBox;\
		} \
		break; \
	} \

QWidget* QDPI_Number::GenerateValueWidget() {
	const QJsonObject& MetaData = GetMetaData();
	switch (GetTypeID()) {
		Q_DETAIL_FOR_EACH_NUMBER_TYPE(Q_DETAIL_NUMBER_SWITCH_CASE, Q_DETAIL_NUMBER_SWITCH_CASE)
	default:
		return nullptr;
		break;
	}
	return nullptr;
}
