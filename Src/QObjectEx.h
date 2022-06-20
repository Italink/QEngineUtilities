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
	MetaData.mPropertyMap[#PropertyName]["Min"] = Min; \
	MetaData.mPropertyMap[#PropertyName]["Max"] = Max; 

#define Q_META_CATEGORY(CategoryName,...) \
	MetaData.mCategoryList<<#CategoryName; \
	for(const QString& PropertyName :QString(#__VA_ARGS__).split(',')){ \
		MetaData.mPropertyMap[PropertyName.trimmed()]["Category"] = #CategoryName; \
	}

#define Q_META_STRING_AS_LINE(PropertyName,PlaceholderText) \
	MetaData.mPropertyMap[#PropertyName]["Type"] = "Line";  \
	MetaData.mPropertyMap[#PropertyName]["PlaceholderText"] = PlaceholderText; 

#define Q_META_STRING_AS_MULTI_LINE(PropertyName,WidgetHeight,PlaceholderText) \
	MetaData.mPropertyMap[#PropertyName]["Type"] = "MultiLine";  \
	MetaData.mPropertyMap[#PropertyName]["Height"] = WidgetHeight; \
	MetaData.mPropertyMap[#PropertyName]["PlaceholderText"] = PlaceholderText; 

#define Q_META_STRING_AS_FILE_PATH(PropertyName) \
	MetaData.mPropertyMap[#PropertyName]["Type"] = "FilePath";  

#define Q_META_STRING_AS_COMBO(PropertyName,...) \
	MetaData.mPropertyMap[#PropertyName]["Type"] = "Combo";  \
	MetaData.mPropertyMap[#PropertyName]["ComboList"] = QJsonArray::fromStringList(QString(#__VA_ARGS__).split(','));  

struct QObjectMetaData{
	QHash<QString, QJsonObject> mPropertyMap;
	QList<QString> mCategoryList;
};
