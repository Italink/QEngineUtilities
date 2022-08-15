#include "QDPI_Number.h"
#include "Widgets\QNumberBox.h"

#define Q_DETAIL_NUMBER_SWITCH_CASE(Type) \
	if(type == 	QMetaType::fromType<Type>()){	\
		if (MetaData.contains("Min") && MetaData.contains("Max")) { \
			QNumberBox* numberBox = new QNumberBox( \
				GetValue().value<Type>(), \
				true, \
				MetaData["Min"].value<Type>(), \
				MetaData["Max"].value<Type>() \
			); \
			GetHandler()->Bind(numberBox, &QNumberBox::AsValueChanged,\
			[numberBox]() {\
				return numberBox->GetVar();\
			},\
			[numberBox](QVariant var) {\
				numberBox->SetVar(var);\
			}\
			);\
			return numberBox; \
		} \
		else { \
			QNumberBox* numberBox = new QNumberBox(GetValue().value<Type>()); \
			GetHandler()->Bind(numberBox, &QNumberBox::AsValueChanged,\
			[numberBox]() {\
				return numberBox->GetVar();\
			},\
			[numberBox](QVariant var) {\
				numberBox->SetVar(var);\
			}\
			);\
			return numberBox;\
		} \
	} \

QWidget* QDPI_Number::GenerateValueWidget() {
	const QVariantHash& MetaData = GetMetaData();
	QMetaType type = GetHandler()->GetType();
	Q_DETAIL_FOR_EACH_NUMBER_TYPE(Q_DETAIL_NUMBER_SWITCH_CASE, else Q_DETAIL_NUMBER_SWITCH_CASE)
	return nullptr;
}
