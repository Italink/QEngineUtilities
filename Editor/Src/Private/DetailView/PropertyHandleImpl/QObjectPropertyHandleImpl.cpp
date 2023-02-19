#include "DetailView/PropertyHandleImpl/QObjectPropertyHandleImpl.h"

#include <qsequentialiterable.h>

#include "DetailView/QPropertyHandle.h"
#include "QRegularExpression"
#include "QMetaProperty"

QObjectPropertyHandleImpl::QObjectPropertyHandleImpl(QPropertyHandle* InHandle)
	:IPropertyHandleImpl(InHandle) {
	mObjectHolder = mHandle->GetValue();
	QMetaType metaType = mHandle->GetType();
	QRegularExpression reg("(QSharedPointer|std::shared_ptr|shared_ptr)\\<(.+)\\>");
	QRegularExpressionMatch match = reg.match(metaType.name());
	QStringList matchTexts = match.capturedTexts();
	if (!matchTexts.isEmpty()) {
		QMetaType innerMetaType = QMetaType::fromName((matchTexts.back() + "*").toLocal8Bit());
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

void QObjectPropertyHandleImpl::RefreshObjectPtr() {
	mObjectHolder = mHandle->GetValue();
	if (mObjectHolder.isValid()) {
		if (mMetaObject->inherits(&QObject::staticMetaObject)) {
			QObject* objectPtr = mObjectHolder.value<QObject*>();
			mObjectPtr = objectPtr;
			mOwnerObject = objectPtr;
		}
		else {
			void* ptr = mObjectHolder.data();
			if (bIsPointer)
				ptr = *(void**)mObjectHolder.data();
			mObjectPtr = ptr;
		}
	}
}

QPropertyHandle* QObjectPropertyHandleImpl::FindChildHandle(const QString& inSubName) {
	return QPropertyHandle::Find(mHandle->parent(), inSubName);
}

QWidget* QObjectPropertyHandleImpl::GenerateValueWidget() {
	return IPropertyHandleImpl::GenerateValueWidget();
}

void QObjectPropertyHandleImpl::GenerateChildrenRow(QRowLayoutBuilder* Builder) {
	RefreshObjectPtr();
	IDetailLayoutBuilder::ObjectContext Context;
	Context.MetaObject = mMetaObject;
	Context.ObjectPtr = mObjectPtr;
	Context.OwnerObject = mHandle->parent();
	Context.PrePath = mHandle->GetPath();
	Builder->AddObject(Context);
}

QPropertyHandle* QObjectPropertyHandleImpl::CreateChildHandle(const QString& inSubName) {
	QPropertyHandle* handle = nullptr;
	if (mObjectPtr == nullptr)
		return handle;
	for(int i = 0;i< mMetaObject->propertyCount();i++){
		QMetaProperty prop = mMetaObject->property(i);
		if(prop.name() == inSubName){
			if(mObjectPtr == mOwnerObject){
				handle = new QPropertyHandle(
					mOwnerObject,
					prop.metaType(),
					inSubName,
					[this, prop]() {
						return prop.read(mOwnerObject);
					},
					[this, prop](QVariant var) {
						prop.write(mOwnerObject, var);
					},
					mHandle->GetMetaData()
				);
			}
			else{
				handle = new QPropertyHandle(
					mOwnerObject,
					prop.metaType(),
					inSubName,
					[this, prop]() {
					return prop.readOnGadget(mObjectPtr);
				},
					[this, prop](QVariant var) {
					prop.writeOnGadget(mObjectPtr, var);
					mHandle->SetValue(mObjectHolder);
					RefreshObjectPtr();
				},
					mHandle->GetMetaData()
					);
			}
			break;

		}
	}
	return handle;
}

