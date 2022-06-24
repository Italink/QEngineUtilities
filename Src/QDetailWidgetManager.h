#ifndef QDetailWidgetManager_h__
#define QDetailWidgetManager_h__

#include "QMetaType"
#include "QHash"
#include <functional>
#include "Items\QDetailWidgetPropertyItem.h"
#include "QMap"

class QDetailWidgetManager {
public:
	using PropertyItemCreator = std::function<QDetailWidgetPropertyItem* ()>;
	using PropertyItemFilter = std::function<bool(QPropertyHandler::TypeId)>;

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
		PropertyItemFilter filter= [](QPropertyHandler::TypeId inType) { 
			return PropertyItemType::FilterType(inType); 
		};
		PropertyItemCreator creator = []() {
			return new PropertyItemType();
		};
		mPropertyItemFilterList << QPair<PropertyItemFilter, PropertyItemCreator>{filter, creator};
	}

	const QHash<QPropertyHandler::TypeId, QDetailWidgetManager::PropertyItemCreator>& GetPropertyItemCreatorMap() const { return mPropertyItemCreatorMap; }

	const QList<QPair<QDetailWidgetManager::PropertyItemFilter, QDetailWidgetManager::PropertyItemCreator>>& GetPropertyItemFilterList() const { return mPropertyItemFilterList; }

protected:
	void RegisterBuiltIn();

	QHash<QPropertyHandler::TypeId, PropertyItemCreator> mPropertyItemCreatorMap;

	QList<QPair<PropertyItemFilter, PropertyItemCreator>>  mPropertyItemFilterList;
};


#endif // QDetailWidgetManager_h__
