#ifndef QDetailWidgetManager_h__
#define QDetailWidgetManager_h__

#include "QMetaType"
#include "QHash"
#include <functional>
#include "Customization/Item/QDetailWidgetPropertyItem.h"
#include "Instance/QInstanceDetail.h"
#include "QMap"

class QDetailWidgetManager {
public:
	using PropertyTypeFilter = std::function<bool(QPropertyHandler::TypeId)>;
	using PropertyItemCreator = std::function<QDetailWidgetPropertyItem* ()>;

	using InstanceTypeFilter = std::function<bool(const QSharedPointer<QInstance>& )>;
	using InstanceDetailCreator = std::function< QInstanceDetail*()>;

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
		PropertyTypeFilter filter= [](QPropertyHandler::TypeId inType) { 
			return PropertyItemType::FilterType(inType); 
		};
		PropertyItemCreator creator = []() {
			return new PropertyItemType();
		};
		mPropertyItemFilterList << QPair<PropertyTypeFilter, PropertyItemCreator>{filter, creator};
	}

	template<typename InstanceDetailType>
	void RegisterInstanceFilter() {
		InstanceTypeFilter filter = [](const QSharedPointer<QInstance>& inInstance) {
			return InstanceDetailType::Filter(inInstance);
		};
		InstanceDetailCreator creator = []() {
			return new InstanceDetailType();
		};
		mInstanceFilterList << QPair<InstanceTypeFilter, InstanceDetailCreator>{filter, creator};
	}

	const QHash<QPropertyHandler::TypeId,PropertyItemCreator>& GetPropertyItemCreatorMap() const { return mPropertyItemCreatorMap; }
	const QList<QPair<PropertyTypeFilter,PropertyItemCreator>>& GetPropertyItemFilterList() const { return mPropertyItemFilterList; }
	const QList<QPair<InstanceTypeFilter, InstanceDetailCreator>>& GetInstanceDetailFilterList() const { return mInstanceFilterList; }
protected:
	void RegisterBuiltIn();

private:
	QHash<QPropertyHandler::TypeId, PropertyItemCreator> mPropertyItemCreatorMap;
	QList<QPair<PropertyTypeFilter, PropertyItemCreator>>  mPropertyItemFilterList;
	QList<QPair<InstanceTypeFilter, InstanceDetailCreator>>  mInstanceFilterList;
};

#endif // QDetailWidgetManager_h__
