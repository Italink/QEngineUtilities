#include "DetailView/PropertyHandleImpl/IPropertyHandleImpl.h"
#include "Widgets/QElideLabel.h"
#include "DetailView/QPropertyHandle.h"
#include "QBoxLayout"
#include "DetailView/QDetailViewManager.h"

QPropertyHandle* IPropertyHandleImpl::CreateChildHandle(const QString& inSubName) {
	return nullptr;
}

QWidget* IPropertyHandleImpl::GenerateNameWidget() {
	return new QElideLabel(mHandle->objectName());
}

QWidget* IPropertyHandleImpl::GenerateValueWidget() {
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(QDetailViewManager::Instance()->GetCustomPropertyValueWidget(mHandle));
	mHandle->GenerateAttachButtonWidget(valueContentLayout);
	return valueContent;
}

