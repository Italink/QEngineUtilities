#include "QInstanceDetail.h"
#include "QMetaProperty"
#include "QMetaType"
#include "Core\QDetailWidgetPrivate.h"
#include "Customization\QDetailWidgetManager.h"
#include "Customization\Item\QDetailWidgetCategoryItem.h"
#include "Customization\Item\QDetailWidgetItem.h"

QInstanceDetail* CreateInternal(QSharedPointer<QInstance> inInstance) {
	QInstanceDetail* instanceDetail = nullptr;
	for (auto& filter : QDetailWidgetManager::instance()->GetInstanceDetailFilterList()) {
		if (filter.first(inInstance)) {
			instanceDetail = filter.second();
			break;
		}
	}
	if (instanceDetail == nullptr)
		instanceDetail = new QInstanceDetail;
	return instanceDetail;
}


QInstanceDetail* QInstanceDetail::FindOrCreate(QSharedPointer<QInstance> inInstance, QDetailTreeWidget* inWidget) {
	for (auto instanceDetail : inInstance->GetOuterObject()->findChildren<QInstanceDetail*>()) {
		if (instanceDetail->mWidget == inWidget) {
			return instanceDetail;
		}
	}
	QInstanceDetail* instanceDetail = CreateInternal(inInstance);
	instanceDetail->Initialize(inInstance, inWidget);
	return instanceDetail;
}

QInstanceDetail* QInstanceDetail::FindOrCreate(QSharedPointer<QInstance> inInstance, QDetailWidgetItem* inRoot) {
	for (auto instanceDetail : inInstance->GetOuterObject()->findChildren<QInstanceDetail*>()) {
		if (instanceDetail->mRoot == inRoot) {
			return instanceDetail;
		}
	}
	QInstanceDetail* instanceDetail = CreateInternal(inInstance);
	instanceDetail->Initialize(inInstance, inRoot);
	return instanceDetail;
}

void QInstanceDetail::Initialize( QSharedPointer<QInstance> inInstance, QDetailTreeWidget* inWidget) {
	mInstance = inInstance;
	mWidget = inWidget;
	setParent(inInstance->GetOuterObject());
}

void QInstanceDetail::Initialize( QSharedPointer<QInstance> inInstance, QDetailWidgetItem* inRoot) {
	mInstance = inInstance;
	mRoot = inRoot;
	setParent(inInstance->GetOuterObject());
}

void QInstanceDetail::Build() {
	PreBuildCategory();
	for (int i = 0; i < mInstance->GetMetaObject()->propertyCount(); i++) {
		QMetaProperty prop = mInstance->GetMetaObject()->property(i);
		QPropertyHandler* propHandler = mInstance->CreatePropertyHandler(prop);
		AddProperty(propHandler);
	}
}

void QInstanceDetail::AddProperty(QPropertyHandler* inPropertyHandler) {
	QString categoryName = inPropertyHandler->GetMetaData("Category").toString();
	if (mInstance->GetCategoryMap()[categoryName])
		return;
	QDetailWidgetPropertyItem* item = QDetailWidgetPropertyItem::Create(inPropertyHandler);
	if (item) {
		if (mInstance->GetMetaData("CategoryEnabled").toBool()) {
			if (categoryName.isEmpty())
				categoryName = mInstance->GetOuterObject()->objectName();
			if (categoryName.isEmpty())
				categoryName = "Common";
			QDetailWidgetCategoryItem* categoryItem = FindOrAddCategory(categoryName);
			item->AttachTo(categoryItem);
		}
		else {
			if (mWidget)
				item->AttachTo(mWidget);
			else
				item->AttachTo(mRoot);
		}
	}
}

void QInstanceDetail::PreBuildCategory() {
	if (mInstance->GetMetaData("CategoryEnabled").toBool()) {
		for (auto category = mInstance->GetCategoryMap().begin(); category != mInstance->GetCategoryMap().end();++category) {
			if(category.value())
				FindOrAddCategory(category.key());
		}
	}
}

QDetailWidgetCategoryItem* QInstanceDetail::FindOrAddCategory(QString inName) {
	if (mWidget) {
		for (int i = 0; i < mWidget->topLevelItemCount(); i++) {
			QTreeWidgetItem* item = mWidget->topLevelItem(i);
			if (item->text(0) == inName)
				return static_cast<QDetailWidgetCategoryItem*>(item);
		}
		QDetailWidgetCategoryItem* newItem = new QDetailWidgetCategoryItem(inName);
		newItem->setSizeHint(0, { 25,25 });
		newItem->AttachTo(mWidget);
		return newItem;
	}
	else {
		for (int i = 0; i < mRoot->childCount(); i++) {
			QTreeWidgetItem* item = mRoot->child(i);
			if (item->text(0) == inName)
				return static_cast<QDetailWidgetCategoryItem*>(item);
		}
		QDetailWidgetCategoryItem* newItem = new QDetailWidgetCategoryItem(inName);
		newItem->setSizeHint(0, { 25,25 });
		newItem->AttachTo(mRoot);
		return newItem;
	}
}
