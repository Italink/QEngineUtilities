#include "QInstance.h"
#include "QPropertyHandler.h"
#include "QMetaDataDefine.h"

QSharedPointer<QInstance_Object> QInstance::CreateObjcet(QObject* inObject) {
	QSharedPointer<QInstance_Object> instance = QSharedPointer<QInstance_Object>::create(inObject);
	instance->LoadMetaData();
	return instance;
}

QSharedPointer<QInstance_Gadget> QInstance::CreateGadget(void* inPtr, const QMetaObject* inMetaObject) {
	QSharedPointer<QInstance_Gadget> instance = QSharedPointer<QInstance_Gadget>::create(inPtr,inMetaObject);
	instance->LoadMetaData();
	return instance;
}

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

QVariant QInstance::GetMetaData(const QString& inKey) {
	return mMetaData.mInstanceMetaData.value(inKey);
}

void QInstance::SetMetaData(const QString& inKey, QVariant inVar) {
	mMetaData.mInstanceMetaData[inKey] = inVar;
}

const QHash<QString, bool>& QInstance::GetCategoryMap() {
	return mMetaData.mCategories;;
}

const QHash<QString, QVariantHash>& QInstance::GetPropertiesMetaData() {
	return mMetaData.mPropertiesMetaData;
}

void QInstance::LoadMetaData() {
	for (int i = 0; i < GetMetaObject()->methodCount(); i++) {
		QMetaMethod method = GetMetaObject()->method(i);
		if (QString(method.name()).endsWith("_GetMetaData")) {
			QMetaData MetaData;
			if (Invoke(method, Q_RETURN_ARG(QMetaData, MetaData))) {
				for (auto PropertyIter = MetaData.mPropertiesMetaData.begin(); PropertyIter != MetaData.mPropertiesMetaData.end(); ++PropertyIter) {
					mMetaData.mPropertiesMetaData[PropertyIter.key()] = PropertyIter.value();
				}
				mMetaData.mCategories.insert(MetaData.mCategories);
				mMetaData.mInstanceMetaData.insert(MetaData.mInstanceMetaData);
			}
		}
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

QPropertyHandler* QInstance::CreatePropertyHandler(const QMetaProperty& inProperty) {
	QPropertyHandler* propHandler = QPropertyHandler::FindOrCreate(
		GetOuterObject(),
		inProperty.metaType(),
		inProperty.name(),
		[this, inProperty]() {return GetProperty(inProperty); },
		[this, inProperty](QVariant var) { SetProperty(inProperty, var); },
		mMetaData.mPropertiesMetaData[inProperty.name()]
	);
	QObject::connect(propHandler, &QPropertyHandler::AsValueChanged,[this]() {
		if (mPropertyChangedCallback)
			mPropertyChangedCallback();
	});
	return propHandler;
}

