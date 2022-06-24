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
	connect(comboBox, &QComboBox::currentTextChanged, this, [this](QString text) {
		SetValue(GetEnumValueByName(text));
	});
	comboBox->setMinimumWidth(90);
	GetContent()->SetNameWidgetByText(GetName());
	GetContent()->SetValueWidget(comboBox);
	treeWidget()->setItemWidget(this, 0, GetContent());
}

