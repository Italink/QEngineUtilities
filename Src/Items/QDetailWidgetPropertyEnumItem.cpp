#include "QDetailWidgetPropertyEnumItem.h"
#include "QMetaType"
#include "QSequentialIterable"
#include "QDetailWidgetManager.h"
#include "QComboBox"
#include "QJsonArray"

bool QDetailWidgetPropertyEnumItem::FilterType(TypeId inID) {
	QMetaType metaType(inID);
	return metaType.flags() & QMetaType::IsEnumeration;
}

int QDetailWidgetPropertyEnumItem::GetEnumValueByName(QString inName)
{
	return mNameToValueMap.value(inName);
}

QString QDetailWidgetPropertyEnumItem::GetKeywords()
{
	return GetName() + mNameToValueMap.keys().join("");
}

QWidget* QDetailWidgetPropertyEnumItem::GenerateValueWidget() {
	return nullptr;
}

void QDetailWidgetPropertyEnumItem::BuildContentAndChildren() {
	QComboBox* comboBox = new QComboBox();
	QJsonArray enumList = GetMetaData()["EnumList"].toArray();
	for (auto enumData : enumList) {
		const QJsonObject& data = enumData.toObject();
		comboBox->addItem(data["Name"].toString());
		mNameToValueMap[data["Name"].toString()] = data["Value"].toInt();
	}
	GetHandler()->Bind(
		comboBox,
		&QComboBox::currentTextChanged,
		[comboBox, this]() {
			return mNameToValueMap.value(comboBox->currentText());
		},
		[comboBox, this](QVariant var) {
			comboBox->setCurrentText(mNameToValueMap.key(var.toInt()));;
		}
	);

	comboBox->setMinimumWidth(90);
	GetContent()->SetNameWidgetByText(GetName());
	GetContent()->SetValueWidget(comboBox);
	treeWidget()->setItemWidget(this, 0, GetContent());
}

