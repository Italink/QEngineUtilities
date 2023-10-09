#include "DetailView/PropertyHandleImpl/QEnumPropertyHandleImpl.h"
#include <QComboBox>
#include "DetailView/QPropertyHandle.h"
#include <QMetaEnum>
#include <QWidget>
#include <QBoxLayout>

QEnumPropertyHandleImpl::QEnumPropertyHandleImpl(QPropertyHandle* InHandle)
	:IPropertyHandleImpl(InHandle) {
	const QMetaObject* metaObj = mHandle->getType().metaObject();
	if (metaObj){
		const QMetaEnum& metaEnum = metaObj->enumerator(metaObj->indexOfEnumerator(QString(mHandle->getType().name()).split("::").last().toLocal8Bit()));
		for (int i = 0; i < metaEnum.keyCount(); i++) {
			mNameToValueMap[metaEnum.key(i)] = metaEnum.value(i);
		}
	}
}

QWidget* QEnumPropertyHandleImpl::generateValueWidget() {
	QComboBox* comboBox = new QComboBox();
	for(auto enumPair :mNameToValueMap.asKeyValueRange()){
		comboBox->addItem(enumPair.first);
	}
	mHandle->bind(
		comboBox,
		&QComboBox::currentTextChanged,
		[comboBox, this]() {
		return mNameToValueMap.value(comboBox->currentText());
	},
		[comboBox, this](QVariant var) {
		comboBox->setCurrentText(mNameToValueMap.key(var.toInt()));
	}
	);
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(comboBox);
	mHandle->generateAttachButtonWidget(valueContentLayout);
	return valueContent;
}
