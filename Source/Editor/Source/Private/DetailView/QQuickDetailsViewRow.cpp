#include "QQuickDetailsViewRow.h"
#include "qqmlcontext.h"
#include "qsequentialiterable.h"
#include <QAssociativeIterable>
#include "QQuickDetailsViewLayoutBuilder.h"
#include "QQuickDetailsViewMananger.h"

void IDetailsViewRow::addChild(QSharedPointer<IDetailsViewRow> inChild)
{
	mChildren << inChild;
	inChild->mParent = this;
}

void IDetailsViewRow::clear()
{
	mChildren.clear();
}

QDetailsViewRow_Property::QDetailsViewRow_Property(QPropertyHandle* inHandle)
    : mHandle(inHandle)
{
    mPropertyTypeCustomization = QQuickDetailsViewManager::Get()->getCustomPropertyType(mHandle->getType());
    if (mHandle->getPropertyType() == QPropertyHandle::Object) {
        auto objectHandle = mHandle->asObject();
        mClassLayoutCustomization = QQuickDetailsViewManager::Get()->getCustomDetailLayout(objectHandle->getMetaObject());
    }
}

void QDetailsViewRow_Root::addChild(QSharedPointer<IDetailsViewRow> inChild)
{
    mChildren << inChild;
}

void QDetailsViewRow_Root::setObject(QObject* inObject)
{
    mObject = inObject;
    mClassLayoutCustomization = QQuickDetailsViewManager::Get()->getCustomDetailLayout(mObject->metaObject());
}

void QDetailsViewRow_Root::attachChildren()
{
	if (mClassLayoutCustomization) {
		QQuickDetailsViewLayoutBuilder builder(this);
		mClassLayoutCustomization->customizeDetails(mObject, &builder);
	}
	else {
		for (int i = 1; i < mObject->metaObject()->propertyCount(); i++) {
			QMetaProperty prop = mObject->metaObject()->property(i);
			QString propertyPath = prop.name();
			QPropertyHandle* handler = QPropertyHandle::FindOrCreate(mObject, propertyPath);
			QSharedPointer<IDetailsViewRow> child(new QDetailsViewRow_Property(handler));
			addChild(child);
			child->attachChildren();
		}
	}
}

void QDetailsViewRow_Property::setupItem(QQuickItem* inParent)
{
	QQuickDetailsViewHeaderRowBuilder builder(inParent);
    if (mPropertyTypeCustomization) {
        mPropertyTypeCustomization->customizeHeader(mHandle, &builder);
        return;
    }
	builder.makePropertyHeader(mHandle);
}

void QDetailsViewRow_Property::attachChildren()
{
    if (mHandle->getPropertyType() == QPropertyHandle::Sequential) {
		QVariant varList = mHandle->getVar();
		QSequentialIterable iterable = varList.value<QSequentialIterable>();
		for (int index = 0; index < iterable.size(); index++) {
			QString path = mHandle->createSubPath(QString::number(index));
			QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
			if (handle) {
				QSharedPointer<IDetailsViewRow> child(new QDetailsViewRow_Property(handle));
                addChild(child);
                child->attachChildren();
			}
		}
    }
	else if (mHandle->getPropertyType() == QPropertyHandle::Associative) {
		QVariant varMap = mHandle->getVar();
		QAssociativeIterable iterable = varMap.value<QAssociativeIterable>();
		for (auto iter = iterable.begin(); iter != iterable.end(); ++iter) {
			QString path = mHandle->createSubPath(iter.key().toString());
			QPropertyHandle* handle = QPropertyHandle::FindOrCreate(mHandle->parent(), path);
			if (handle) {
				QSharedPointer<IDetailsViewRow> child(new QDetailsViewRow_Property(handle));
				addChild(child);
				child->attachChildren();
			}
		}
	}
	else if (mHandle->getPropertyType() == QPropertyHandle::Object) {
		auto objectHandle = mHandle->asObject();
		QObject* object = objectHandle->getObject();
		if (object == nullptr)
			return;
        if (mClassLayoutCustomization) {
            QQuickDetailsViewLayoutBuilder builder(this);
            mClassLayoutCustomization->customizeDetails(object, &builder);
        }
        else {
			for (int i = 1; i < object->metaObject()->propertyCount(); i++) {
				QMetaProperty prop = object->metaObject()->property(i);
				QString propertyPath = mHandle->createSubPath(prop.name());
				QPropertyHandle* handler = QPropertyHandle::FindOrCreate(mHandle->parent(), propertyPath);
				QSharedPointer<IDetailsViewRow> child(new QDetailsViewRow_Property(handler));
				addChild(child);
				child->attachChildren();
			}
        }
	}
    else if (mHandle->getPropertyType() == QPropertyHandle::RawType){
		QQuickDetailsViewLayoutBuilder builder(this);
        if(mPropertyTypeCustomization)
            mPropertyTypeCustomization->customizeChildren(mHandle, &builder);
    }
}

QDetailsViewRow_Custom::QDetailsViewRow_Custom(QQuickItem* inItem)
    : mRowItem(inItem)
{

}

void QDetailsViewRow_Custom::setupItem(QQuickItem* inParent)
{
    mRowItem->setParentItem(inParent);
}
