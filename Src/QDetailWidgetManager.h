#ifndef QDetailWidgetManager_h__
#define QDetailWidgetManager_h__

#include "QMetaType"
#include "QHash"
#include <functional>
#include "Items\QDetailWidgetPropertyItem.h"
#include "QMap"


class QDetailWidgetManager {
public:
	QDetailWidgetManager();

	static QDetailWidgetManager* instance();

	template<typename PropertyItemType>
	void RegisterPropertyItemCreator() {
		QList<int> typeIdList = PropertyItemType::SupportedTypes();
		PropertyItemCreator creator = []() {
			return new PropertyItemType();
		};
		for (int id : typeIdList) {
			mPropertyItemCreatorMap[id] = creator;
		}
	}

	template<typename PropertyItemType>
	void RegisterPropertyItemFilter() {
		PropertyItemFilter filter= [](QDetailWidgetPropertyItem::TypeId inType) { 
			return PropertyItemType::FilterType(inType); 
		};
		PropertyItemCreator creator = []() {
			return new PropertyItemType();
		};
		mPropertyItemFilterList << QPair<PropertyItemFilter, PropertyItemCreator>{filter, creator};
	}
	QDetailWidgetPropertyItem* CreatePropertyItem(QDetailWidgetPropertyItem::TypeId inTypeID, QString inName, QDetailWidgetPropertyItem::Getter inGetter, QDetailWidgetPropertyItem::Setter inSetter, QJsonObject inMetaData);
protected:
	void RegisterBuiltIn();
private:
	using PropertyItemCreator = std::function<QDetailWidgetPropertyItem*()>;
	using PropertyItemFilter = std::function<bool(QDetailWidgetPropertyItem::TypeId)>;

	QHash<QDetailWidgetPropertyItem::TypeId, PropertyItemCreator> mPropertyItemCreatorMap;
	QList<QPair<PropertyItemFilter, PropertyItemCreator>>  mPropertyItemFilterList;
};


#endif // QDetailWidgetManager_h__
