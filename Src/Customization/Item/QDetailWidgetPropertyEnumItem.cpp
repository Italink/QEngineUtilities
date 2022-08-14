#include "QDetailWidgetPropertyEnumItem.h"
#include "QMetaType"
#include "QSequentialIterable"
#include "Customization\QDetailWidgetManager.h"
#include "QComboBox"
#include "QMetaEnum"

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
	QVariant var = GetValue();
	const QMetaObject* metaObj = var.metaType().metaObject();
	if (metaObj) {
		const QMetaEnum& metaEnum = metaObj->enumerator(metaObj->enumeratorOffset());
		QComboBox* comboBox = new QComboBox();
		for (int i = 0; i < metaEnum.keyCount(); i++) {
			comboBox->addItem(metaEnum.key(i));
			mNameToValueMap[metaEnum.key(i)] = metaEnum.value(i);
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
		return comboBox;
	}
	return nullptr;
}
