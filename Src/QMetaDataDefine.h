#include "QJsonObject"
#include "QJsonArray"
#include "QJsonDocument"

#define Q_META_BEGIN(ClassName) \
	Q_INVOKABLE	static QObjectMetaData ClassName##_GetMetaData(){ \
		QObjectMetaData MetaData;		\

#define Q_META_END(...) \
		return MetaData; \
	}
#define Q_META_NUMBER_LIMITED(PropertyName,Min,Max) \
	MetaData.mPropertiesMetaData[#PropertyName]["Min"] = Min; \
	MetaData.mPropertiesMetaData[#PropertyName]["Max"] = Max; 

#define Q_META_CATEGORY(CategoryName,...) \
	MetaData.mCategoryList<<#CategoryName; \
	for(const QString& PropertyName :QString(#__VA_ARGS__).split(',')){ \
		MetaData.mPropertiesMetaData[PropertyName.trimmed()]["Category"] = #CategoryName; \
	}

#define Q_META_STRING_AS_LINE(PropertyName,PlaceholderText) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "Line";  \
	MetaData.mPropertiesMetaData[#PropertyName]["PlaceholderText"] = PlaceholderText; 

#define Q_META_STRING_AS_MULTI_LINE(PropertyName,WidgetHeight,PlaceholderText) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "MultiLine";  \
	MetaData.mPropertiesMetaData[#PropertyName]["Height"] = WidgetHeight; \
	MetaData.mPropertiesMetaData[#PropertyName]["PlaceholderText"] = PlaceholderText; 

#define Q_META_STRING_AS_FILE_PATH(PropertyName) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "FilePath";  

#define Q_META_STRING_AS_COMBO(PropertyName,...) \
	MetaData.mPropertiesMetaData[#PropertyName]["Type"] = "Combo";  \
	MetaData.mPropertiesMetaData[#PropertyName]["ComboList"] = QString(#__VA_ARGS__).split(',');  

struct QObjectMetaData{
	QList<QString> mCategoryList;
	QHash<QString, QVariant> mClassMetaData;
	QHash<QString, QVariantHash> mPropertiesMetaData;
};
