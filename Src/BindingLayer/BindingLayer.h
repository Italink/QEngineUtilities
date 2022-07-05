#ifndef BindingLayer_h__
#define BindingLayer_h__

#include "QObject"
#include "QVariant"
#include "QAssociativeIterable"
#include "QSequentialIterable"

namespace BindingLayer {
	using TypeId = unsigned int;
	using Instance = QObject*;
	using Variant = QVariant;
	template<typename _Ty>
	TypeId GetTypeId() {
		return QMetaTypeId2<_Ty>::qt_metatype_id();
	}
	TypeId GetTypeId(const Variant& inVar);

	Variant GetProperty(Instance inInstance, QString inName);
	void SetProperty(Instance inInstance, QString inName, Variant inVar);

	bool IsAssociative(TypeId inId);
	QList<Variant> GetAssociativeKeyList(const Variant& inAssociative);
	Variant GetAssociativeValue(const Variant& inAssociative,const QString& inKey);
	void SetAssociativeValue(Variant& inAssociative, const QString& inKey,const Variant& inVar);
	bool AssociativeContainKey(const Variant& inAssociative, const Variant& inKey);

	bool IsSequential(TypeId inId);
	unsigned int GetSequentialCount(const Variant& inSequential);
	Variant GetSequentialValue(const Variant& inSequential, int inIndex);
	void SetSequentialValue(Variant& inSequential, int inIndex , const Variant& inVar);
	void PushSequentialValue(Variant& inSequential, const Variant& inVar, bool isBack = true );
	void PopSequentialValue(Variant& inSequential, bool isBack = true);

	bool CanConvert(TypeId inSrc,TypeId inDst);

	bool IsStringType(TypeId inId);
	QString AnyStringToQString(const Variant& inVar);
	Variant QStringToAnyString(const QString& inSrcString, TypeId inDstString);
};

#endif // BindingLayer_h__
