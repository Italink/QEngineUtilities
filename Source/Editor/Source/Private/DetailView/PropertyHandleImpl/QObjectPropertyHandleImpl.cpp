#include "DetailView/PropertyHandleImpl/QObjectPropertyHandleImpl.h"
#include <qsequentialiterable.h>
#include "DetailView/QPropertyHandle.h"
#include "QRegularExpression"
#include <QMetaProperty>
#include <QThread>

QObjectPropertyHandleImpl::QObjectPropertyHandleImpl(QPropertyHandle* InHandle)
	:IPropertyHandleImpl(InHandle) {
	mObjectHolder = mHandle->getValue();
	QMetaType metaType = mHandle->getType();
	QRegularExpression reg("QSharedPointer\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(metaType.name());
	QStringList matchTexts = match.capturedTexts();
	if (!matchTexts.isEmpty()) {
		QMetaType innerMetaType = QMetaType::fromName((matchTexts.back()).toLocal8Bit());
		mMetaObject = innerMetaType.metaObject();
		const void* ptr = *(const void**)mObjectHolder.data();
		bIsSharedPointer = true;
		bIsPointer = true;
		if (ptr) {
			mObjectHolder = QVariant(innerMetaType, mObjectHolder.data());
		}
		else {
			mObjectHolder = QVariant();
		}
	}
	else{
		bIsPointer = metaType.flags().testFlag(QMetaType::IsPointer);
		mMetaObject = metaType.metaObject();
	}
	mOwnerObject = mHandle->parent();
}

void QObjectPropertyHandleImpl::refreshObjectPtr() {
	mObjectHolder = mHandle->getValue();
	if (mObjectHolder.isValid()) {
		if (mMetaObject->inherits(&QObject::staticMetaObject)) {
			QObject* objectPtr = mObjectHolder.value<QObject*>();
			if (objectPtr) {
				mMetaObject = objectPtr->metaObject();
			}
			mObjectPtr = objectPtr;
			mOwnerObject = objectPtr;
			if (mOwnerObject) {
				mOwnerObject->installEventFilter(mHandle);
			}
		}
		else {
			void* ptr = mObjectHolder.data();
			if (bIsPointer)
				ptr = *(void**)mObjectHolder.data();
			mObjectPtr = ptr;
		}
	}
}

QPropertyHandle* QObjectPropertyHandleImpl::findChildHandle(const QString& inSubName) {
	return QPropertyHandle::Find(mHandle->parent(), inSubName);
}

QWidget* QObjectPropertyHandleImpl::generateValueWidget() {
	return IPropertyHandleImpl::generateValueWidget();
}

void QObjectPropertyHandleImpl::generateChildrenRow(QRowLayoutBuilder* Builder) {
	refreshObjectPtr();
	IDetailLayoutBuilder::ObjectContext Context;
	Context.MetaObject = mMetaObject;
	Context.ObjectPtr = mObjectPtr;
	Context.OwnerObject = mHandle->parent();
	Context.PrePath = mHandle->getPath();
	Builder->addObject(Context);
}

QPropertyHandle* QObjectPropertyHandleImpl::createChildHandle(const QString& inSubName) {
	QPropertyHandle* handle = nullptr;
	if (mObjectPtr == nullptr)
		return handle;
	QString propertyPath = mMetaObject->inherits(&QObject::staticMetaObject) ? inSubName : mHandle->getSubPath(inSubName);
	for(int i = 0;i< mMetaObject->propertyCount();i++){
		QMetaProperty prop = mMetaObject->property(i);
		if(prop.name() == inSubName){
			if(mObjectPtr == mOwnerObject){
				handle = new QPropertyHandle(
					mOwnerObject,
					prop.metaType(),
					propertyPath,
					[this, prop]() {
						return prop.read(mOwnerObject);
					},
					[this, prop](QVariant var) {
						prop.write(mOwnerObject, var);
					}
				);
			}
			else{
				handle = new QPropertyHandle(
					mOwnerObject,
					prop.metaType(),
					propertyPath,
					[this, prop]() {
						return prop.readOnGadget(mObjectPtr);
					},
					[this, prop](QVariant var) {
						prop.writeOnGadget(mObjectPtr, var);
						mHandle->setValue(mObjectHolder);
						refreshObjectPtr();
					}
					);
			}
			break;

		}
	}
	return handle;
}

