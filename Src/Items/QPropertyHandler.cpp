#include "QPropertyHandler.h"
#include "QMetaObject"
#include "QMetaProperty"

QPropertyHandler::QPropertyHandler(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter)
	:QObject(inParent)
{
	mTypeID = inTypeID;
	mName = inName;
	mGetter = inGetter;
	mSetter = inSetter;
	mInitialValue = inGetter();
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QObject* inObject, QString inPropertyName)
{
	int index = inObject->metaObject()->indexOfProperty(inPropertyName.toLocal8Bit());
	QMetaProperty metaProperty = inObject->metaObject()->property(index);
	return FindOrCreate(inObject,
		metaProperty.typeId(),
		inPropertyName,
		[Object = inObject, metaProperty]() {return metaProperty.read(Object); },
		[Object = inObject, metaProperty](QVariant var) { metaProperty.write(Object, var); }
	);
}

QPropertyHandler* QPropertyHandler::FindOrCreate(QObject* inParent, TypeId inTypeID, QString inName, Getter inGetter, Setter inSetter)
{
	for (QObject* child : inParent->children()) {
		QPropertyHandler* handler = qobject_cast<QPropertyHandler*>(child);
		if (handler && handler->GetName() == inName) {
			return handler;
		}
	}
	QPropertyHandler* handler = new QPropertyHandler(
		inParent,
		inTypeID,
		inName,
		inGetter,
		inSetter
	);
	return handler;
}

void QPropertyHandler::SetValue(QVariant value)
{
	QVariant last = GetValue();
	if (last != value) {
		if (mInitialValue.metaType().flags() & QMetaType::IsEnumeration ) {
			mIsChanged = value.toInt() != mInitialValue.toInt();
		}
		else {
			mIsChanged = (value != mInitialValue);
		}
		mSetter(value);
		for (auto& binder : mBinderList) {
			QVariant var = binder.mGetter();
			if (var != value) {
				binder.mSetter(value);
			}
		}
		Q_EMIT AsValueChanged();
	}
}

QVariant QPropertyHandler::GetValue()
{
	return mGetter();
}

void QPropertyHandler::ResetValue() {
	if (mIsChanged) {
		SetValue(mInitialValue);
	}
}

QPropertyHandler::TypeId QPropertyHandler::GetTypeID() {
	return mTypeID;
}

QString QPropertyHandler::GetName()
{
	return mName;
}

