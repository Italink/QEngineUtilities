#include "BindingLayer.h"

BindingLayer::TypeId BindingLayer::GetTypeId(const Variant& inVar)
{
	return inVar.typeId();
}

BindingLayer::Variant BindingLayer::GetProperty(Instance inInstance, QString inName)
{
	return inInstance->property(inName.toLocal8Bit());
}

void BindingLayer::SetProperty(Instance inInstance, QString inName, Variant inVar)
{
	inInstance->setProperty(inName.toLocal8Bit(), inVar);
}

bool BindingLayer::IsAssociative(TypeId inId)
{
	QMetaType metaType(inId);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

QList<BindingLayer::Variant> BindingLayer::GetAssociativeKeyList(const Variant& inAssociative)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	QList<Variant> keyList;
	return keyList;
}

BindingLayer::Variant BindingLayer::GetAssociativeValue(const Variant& inAssociative, const QString& inKey)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	return iterable.value(inKey);
}

void BindingLayer::SetAssociativeValue(Variant& inAssociative, const QString& inKey, const Variant& inVar)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	QMetaAssociation metaAssociation = iterable.metaContainer();
	QtPrivate::QVariantTypeCoercer keyCoercer;
	QtPrivate::QVariantTypeCoercer mappedCoercer;
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	const void* dataPtr = mappedCoercer.coerce(inVar, inVar.metaType());
	metaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(inKey, metaAssociation.keyMetaType()), dataPtr);
}

bool BindingLayer::AssociativeContainKey(const Variant& inAssociative, const Variant& inKey)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	return iterable.containsKey(inKey);
}

bool BindingLayer::IsSequential(TypeId inId)
{
	QMetaType metaType(inId);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

unsigned int BindingLayer::GetSequentialCount(const Variant& inSequential)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	return iterable.size();
}

BindingLayer::Variant BindingLayer::GetSequentialValue(const Variant& inSequential, int inIndex)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
	return iterable.at(inIndex);
}

void BindingLayer::SetSequentialValue(Variant& inSequential, int inIndex, const Variant& inVar)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	const void* dataPtr = coercer.coerce(inVar, inVar.metaType());
	metaSequence.setValueAtIndex(containterPtr, inIndex, dataPtr);
}

void BindingLayer::PushSequentialValue(Variant& inSequential, const Variant& inVar, bool isBack /*= true */)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	QMetaType valueType(inVar.typeId());
	const void* dataPtr = coercer.coerce(inVar, inVar.metaType());
	if (isBack)
		metaSequence.addValueAtEnd(containterPtr, dataPtr);
	else
		metaSequence.addValueAtBegin(containterPtr, dataPtr);
}

void BindingLayer::PopSequentialValue(Variant& inSequential, bool isBack /*= true*/)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	if (isBack)
		metaSequence.removeValueAtEnd(containterPtr);
	else
		metaSequence.removeValueAtBegin(containterPtr);
}

bool BindingLayer::CanConvert(TypeId inSrc, TypeId inDst)
{
	return QMetaType::canConvert(QMetaType(inSrc), QMetaType(inDst));
}

bool BindingLayer::IsStringType(TypeId inId)
{
	return QMetaTypeId2<QString>::qt_metatype_id() == inId;
}

QString BindingLayer::AnyStringToQString(const Variant& inVar)
{
	return inVar.value<QString>();
}

BindingLayer::Variant BindingLayer::QStringToAnyString(const QString& inSrcString, TypeId inDstString)
{
	return inSrcString;
}

