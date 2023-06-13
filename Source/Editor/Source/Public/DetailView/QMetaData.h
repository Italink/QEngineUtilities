#ifndef QMetaData_h__
#define QMetaData_h__

#ifndef QMetaDataDefine_h__
#define QMetaDataDefine_h__

#include "QString"
#include "QVariant"

#define Q_META_DATA_ENABLED 1

inline static QStringList GetParamStringList(const QString& inStr) {
	QStringList result;
	for (QString item : inStr.split(','))
		result << item.trimmed();
	return result;
}

inline static QString GetEnumName(const QString& inStr) {
	return inStr.split("::").back();
}

#if Q_META_DATA_ENABLED
#define Q_META_BEGIN(ClassName) \
	Q_INVOKABLE	static QMetaData ClassName##_GetMetaData(){ \
		QMetaData MetaData;		\

#define Q_META_END(...) \
		return MetaData; \
	}

#define Q_META_LOCAL_ENUM(EnumName,...) \
	QStringList Keys##EnumName =  GetParamStringList(#__VA_ARGS__); \
	QVector<int> Values##EnumName = {__VA_ARGS__}; \
	QHash<QString, int> EnumMap##EnumName;\
	for(int i = 0; i < Keys##EnumName .size() ; i++) EnumMap##EnumName[GetEnumName(Keys##EnumName [i])] = Values##EnumName[i]; \
	MetaData.mClassMetaData[#EnumName] = QVariant::fromValue<>(EnumMap##EnumName); 

#define Q_META_CATEGORY_DEFINE(CategoryName,...) \
	MetaData.mCategories[#CategoryName]; \
	for(const QString& PropertyName :GetParamStringList(#__VA_ARGS__)){ \
		MetaData.mPropertiesMetaData[PropertyName]["Category"] = #CategoryName; \
	}

#define Q_META_CATEGORY_ENABLED() \
	MetaData.mClassMetaData["CategoryEnabled"] = true;

#define Q_META_CATEGORY_VISIBLE(CategoryName,Bool) \
	MetaData.mCategories[#CategoryName] = !Bool; \

#define Q_META_P_VISIBLE(PropertyName,Bool) \
	MetaData.mPropertiesMetaData[#PropertyName]["Visible"] = Bool ;  

#define Q_META_P_ENABLED(PropertyName,Bool) \
	MetaData.mPropertiesMetaData[#PropertyName]["Enabled"] = Bool;

#define Q_META_P_DISPLAYNAME(PropertyName,DisplayName) \
	MetaData.mPropertiesMetaData[#PropertyName]["DisplayName"] = DisplayName;

#define Q_META_P_NUMBER_LIMITED(PropertyName,Min,Max) \
	MetaData.mPropertiesMetaData[#PropertyName]["Min"] = Min; \
	MetaData.mPropertiesMetaData[#PropertyName]["Max"] = Max; 

#define Q_META_P_STRING_AS_LINE(PropertyName,PlaceholderText) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "Line";  \
	MetaData.mPropertiesMetaData[#PropertyName]["PlaceholderText"] = PlaceholderText; 

#define Q_META_P_STRING_AS_MULTI_LINE(PropertyName,WidgetHeight,PlaceholderText) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "MultiLine";  \
	MetaData.mPropertiesMetaData[#PropertyName]["Height"] = WidgetHeight; \
	MetaData.mPropertiesMetaData[#PropertyName]["PlaceholderText"] = PlaceholderText; 

#define Q_META_P_STRING_AS_FILE_PATH(PropertyName) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "FilePath";  

#define Q_META_P_STRING_AS_COMBO(PropertyName,...) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "Combo";  \
	MetaData.mPropertiesMetaData[#PropertyName]["ComboList"] = GetParamStringList(#__VA_ARGS__);  

#define Q_META_P_ARRAY_FIXED_ORDER(PropertyName,Bool) \
	MetaData.mPropertiesMetaData[#PropertyName]["FixedOrder"] = Bool;

#define Q_META_P_ARRAY_FIXED_SIZE(PropertyName,Bool) \
	MetaData.mPropertiesMetaData[#PropertyName]["FixedSize"] = Bool;

#define Q_META_P_MAP_FIXED_KEY(PropertyName,Bool) \
	MetaData.mPropertiesMetaData[#PropertyName]["FixedKey"] = Bool;

#define Q_META_P_MAP_FIXED_SIZE(PropertyName,Bool) \
	MetaData.mPropertiesMetaData[#PropertyName]["FixedSize"] = Bool;

#define Q_META_P_INS_PTR_CHOSE_SUBTYPE_BY_META(PropertyName,...) \
	MetaData.mPropertiesMetaData[#PropertyName]["SubTypeList"] = GetParamStringList(#__VA_ARGS__); 

#else

#define Q_META_EMPTY(...)
#define Q_META_BEGIN(...)
#define Q_META_END(...)

#define Q_META_CATEGORY_DEFINE(...) 
#define Q_META_CATEGORY_ENABLED(...) 
#define Q_META_CATEGORY_VISIBLE(...) 
#define Q_META_P_VISIBLE(...)
#define Q_META_P_ENABLED(...)
#define Q_META_P_ENABLED_BY_CONDITION(...)
#define Q_META_P_NUMBER_LIMITED(...) 
#define Q_META_P_STRING_AS_LINE(...) 
#define Q_META_P_STRING_AS_MULTI_LINE(...) 
#define Q_META_P_STRING_AS_FILE_PATH(...) 
#define Q_META_P_STRING_AS_COMBO(...)
#define Q_META_P_FILE_FILTER(...)
#define Q_META_P_INSTANCE_CHOSE_TYPE(...)
#define Q_META_P_ARRAY_FIXED_ORDER(...)
#define Q_META_P_ARRAY_FIXED_SIZE(...)
#define Q_META_P_MAP_FIXED_KEY(...)
#define Q_META_P_MAP_FIXED_SIZE(...)
#endif

struct QENGINEEDITOR_API QMetaData {
	using IsHide = bool;
	QHash<QString, IsHide> mCategories;
	QVariantHash mClassMetaData;
	QHash<QString, QVariantHash> mPropertiesMetaData;
};

#endif


#endif // QMetaData_h__
