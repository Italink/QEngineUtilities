#include "DetailView/PropertyHandleImpl/IPropertyHandleImpl.h"
#include "DetailView/QDetailViewManager.h"
#include "DetailView/QPropertyHandle.h"
#include "QBoxLayout"
#include "Widgets/QElideLabel.h"

QPropertyHandle* IPropertyHandleImpl::findChildHandle(const QString& inSubName) {
	return QPropertyHandle::Find(mHandle->parent(), mHandle->getSubPath(inSubName));
}

QPropertyHandle* IPropertyHandleImpl::createChildHandle(const QString& inSubName) {
	return nullptr;
}

QWidget* IPropertyHandleImpl::generateNameWidget() {
	return new QElideLabel(mHandle->getName());
}

QWidget* IPropertyHandleImpl::generateValueWidget() {
	QWidget* valueContent = new QWidget;
	valueContent->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	QHBoxLayout* valueContentLayout = new QHBoxLayout(valueContent);
	valueContentLayout->setAlignment(Qt::AlignLeft);
	valueContentLayout->setContentsMargins(10, 2, 10, 2);
	valueContentLayout->setSpacing(2);
	valueContentLayout->addWidget(QDetailViewManager::Instance()->getCustomPropertyValueWidget(mHandle));
	mHandle->generateAttachButtonWidget(valueContentLayout);
	return valueContent;
}

