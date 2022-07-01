#include "BindingLayerBase.h"

BindingLayerBase::TypeId BindingLayerBase::GetTypeId(const Variant& inVar)
{
	return inVar.typeId();
}

BindingLayerBase::Variant BindingLayerBase::GetProperty(Instance inInstance, QString inName)
{
	return inInstance->property(inName.toLocal8Bit());
}

void BindingLayerBase::SetProperty(Instance inInstance, QString inName, Variant inVar)
{
	inInstance->setProperty(inName.toLocal8Bit(), inVar);
}

bool BindingLayerBase::IsAssociative(TypeId inId)
{
	QMetaType metaType(inId);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

QList<BindingLayerBase::Variant> BindingLayerBase::GetAssociativeKeyList(const Variant& inAssociative)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	QList<Variant> keyList;
	return keyList;
}

BindingLayerBase::Variant BindingLayerBase::GetAssociativeValue(const Variant& inAssociative, const QString& inKey)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	return iterable.value(inKey);
}

void BindingLayerBase::SetAssociativeValue(Variant& inAssociative, const QString& inKey, const Variant& inVar)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	QMetaAssociation metaAssociation = iterable.metaContainer();
	QtPrivate::QVariantTypeCoercer keyCoercer;
	QtPrivate::QVariantTypeCoercer mappedCoercer;
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	const void* dataPtr = mappedCoercer.coerce(inVar, inVar.metaType());
	metaAssociation.setMappedAtKey(containterPtr, keyCoercer.coerce(inKey, metaAssociation.keyMetaType()), dataPtr);
}

bool BindingLayerBase::AssociativeContainKey(const Variant& inAssociative, const Variant& inKey)
{
	QAssociativeIterable iterable = inAssociative.value<QAssociativeIterable>();
	return iterable.containsKey(inKey);
}

bool BindingLayerBase::IsSequential(TypeId inId)
{
	QMetaType metaType(inId);
	if (QMetaType::canConvert(metaType, QMetaType::fromType<QVariantMap>())) {
		return true;
	}
	return false;
}

unsigned int BindingLayerBase::GetSequentialCount(const Variant& inSequential)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	return iterable.size();
}

BindingLayerBase::Variant BindingLayerBase::GetSequentialValue(const Variant& inSequential, int inIndex)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	QMetaType valueMetaType = iterable.metaContainer().valueMetaType();
	return iterable.at(inIndex);
}

void BindingLayerBase::SetSequentialValue(Variant& inSequential, int inIndex, const Variant& inVar)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	QtPrivate::QVariantTypeCoercer coercer;
	const void* dataPtr = coercer.coerce(inVar, inVar.metaType());
	metaSequence.setValueAtIndex(containterPtr, inIndex, dataPtr);
}

void BindingLayerBase::PushSequentialValue(Variant& inSequential, const Variant& inVar, bool isBack /*= true */)
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

void BindingLayerBase::PopSequentialValue(Variant& inSequential, bool isBack /*= true*/)
{
	QSequentialIterable iterable = inSequential.value<QSequentialIterable>();
	const QMetaSequence metaSequence = iterable.metaContainer();
	void* containterPtr = const_cast<void*>(iterable.constIterable());
	if (isBack)
		metaSequence.removeValueAtEnd(containterPtr);
	else
		metaSequence.removeValueAtBegin(containterPtr);
}

bool BindingLayerBase::CanConvert(TypeId inSrc, TypeId inDst)
{
	return QMetaType::canConvert(QMetaType(inSrc), QMetaType(inDst));
}

bool BindingLayerBase::IsStringType(TypeId inId)
{
	return QMetaTypeId2<QString>::qt_metatype_id() == inId;
}

QString BindingLayerBase::AnyStringToQString(const Variant& inVar)
{
	return inVar.value<QString>();
}

BindingLayerBase::Variant BindingLayerBase::QStringToAnyString(const QString& inSrcString, TypeId inDstString)
{
	return inSrcString;
}

