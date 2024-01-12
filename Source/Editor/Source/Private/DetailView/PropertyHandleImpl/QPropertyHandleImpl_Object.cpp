#include "QPropertyHandleImpl_Object.h"
#include <qsequentialiterable.h>
#include <QRegularExpression>
#include "QPropertyHandle.h"
#include <QMetaProperty>

QPropertyHandleImpl_Object::QPropertyHandleImpl_Object(QPropertyHandle* inHandle)
	:IPropertyHandleImpl(inHandle) {
	mObjectHolder = mHandle->getVar();
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
	refreshObjectPtr();
}

QObject* QPropertyHandleImpl_Object::getObject()
{
	if(mMetaObject->inherits(&QObject::staticMetaObject))
		return (QObject*)mObjectPtr;
	return nullptr;
}

QObject* QPropertyHandleImpl_Object::getOwnerObject()
{
	return mOwnerObject;
}

const QMetaObject* QPropertyHandleImpl_Object::getMetaObject() const
{
	return mMetaObject;
}

void QPropertyHandleImpl_Object::refreshObjectPtr() {
	mObjectHolder = mHandle->getVar();
	if (mObjectHolder.isValid()) {
		if (mMetaObject->inherits(&QObject::staticMetaObject)) {
			QObject* objectPtr = mObjectHolder.value<QObject*>();
			if (objectPtr) {
				mMetaObject = objectPtr->metaObject();
			}
			mObjectPtr = objectPtr;
			mOwnerObject = objectPtr;
			if (mOwnerObject) {
				QMetaObject::invokeMethod(mOwnerObject, std::bind(&QObject::moveToThread, mOwnerObject, mHandle->thread()));
				QMetaObject::invokeMethod(mOwnerObject, std::bind(&QObject::installEventFilter, mOwnerObject, mHandle));
				//mOwnerObject->installEventFilter(mHandle);
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

QPropertyHandle* QPropertyHandleImpl_Object::findChildHandle(const QString& inSubName) {
	return QPropertyHandle::Find(mHandle->parent(), inSubName);
}

QQuickItem* QPropertyHandleImpl_Object::createValueEditor(QQuickItem* inParent)
{
	return nullptr;
}

QPropertyHandle* QPropertyHandleImpl_Object::createChildHandle(const QString& inSubName) {
	QPropertyHandle* handle = nullptr;
	if (mObjectPtr == nullptr)
		return handle;
	QString propertyPath = mMetaObject->inherits(&QObject::staticMetaObject) ? inSubName : mHandle->createSubPath(inSubName);
	for(int i = 0;i< mMetaObject->propertyCount();i++){
		QMetaProperty prop = mMetaObject->property(i);
		if(prop.name() == inSubName){
			if(mObjectPtr == mOwnerObject){
				handle = QPropertyHandle::Create(
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
				handle = QPropertyHandle::Create(
					mOwnerObject,
					prop.metaType(),
					propertyPath,
					[this, prop]() {
						return prop.readOnGadget(mObjectPtr);
					},
					[this, prop](QVariant var) {
						prop.writeOnGadget(mObjectPtr, var);
						mHandle->setVar(mObjectHolder);
						refreshObjectPtr();
					}
					);
			}
			break;

		}
	}
	return handle;
}

