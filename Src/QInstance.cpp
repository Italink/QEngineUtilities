#include "QInstance.h"

QInstance_Gadget::QInstance_Gadget(void* inPtr,const QMetaObject* inMetaObject)
	:mPtr(inPtr)
	,mMetaObject(inMetaObject)
{
	mOuter = mOuterCacheMap.value(mPtr,nullptr);
	if (mOuter == nullptr) {
		mOuter = new QObject();
		mOuterCacheMap[mPtr] = mOuter;
	}
}

QInstance_Gadget::~QInstance_Gadget()
{
}

const QMetaObject* QInstance_Gadget::GetMetaObject()
{
	return mMetaObject;
}

QVariant QInstance_Gadget::GetProperty(const QMetaProperty& inProperty)
{
	return inProperty.readOnGadget(mPtr);
}

void QInstance_Gadget::SetProperty(const QMetaProperty& inProperty, QVariant inVar)
{
	inProperty.writeOnGadget(mPtr, inVar);
}

QObject* QInstance_Gadget::GetOuterObject()
{
	return mOuter;
}

bool QInstance_Gadget::Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue)
{
	return inMethod.invokeOnGadget(mPtr, returnValue);
}

QInstance_Object::QInstance_Object(QObject* inObject)
	:mObject(inObject)
{
}

const QMetaObject* QInstance_Object::GetMetaObject()
{
	return mObject->metaObject();
}

QVariant QInstance_Object::GetProperty(const QMetaProperty& inProperty)
{
	return inProperty.read(mObject);
}

void QInstance_Object::SetProperty(const QMetaProperty& inProperty, QVariant inVar)
{
	inProperty.write(mObject, inVar);
}

QObject* QInstance_Object::GetOuterObject()
{
	return mObject;
}

bool QInstance_Object::Invoke(QMetaMethod& inMethod, QGenericReturnArgument returnValue)
{
	return inMethod.invoke(mObject, returnValue);
}
