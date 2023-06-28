#include "DetailView/PropertyHandleImpl/IPropertyHandleImpl.h"
#include "DetailView/QDetailViewManager.h"
#include "DetailView/QPropertyHandle.h"
#include "QBoxLayout"
#include "Widgets/QElideLabel.h"

QPropertyHandle* IPropertyHandleImpl::FindChildHandle(const QString& inSubName) {
	return QPropertyHandle::Find(mHandle->parent(), mHandle->GetSubPath(inSubName));
}

QPropertyHandle* IPropertyHandleImpl::CreateChildHandle(const QString& inSubName) {
	return nullptr;
}

QWidget* IPropertyHandleImpl::GenerateNameWidget() {
	return new QElideLabel(mHandle->GetName());
}

QWidget* IPropertyHandleImpl::GenerateValueWidget() {
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(QDetailViewManager::Instance()->GetCustomPropertyValueWidget(mHandle));
	mHandle->GenerateAttachButtonWidget(valueContentLayout);
	return valueContent;
}

