#ifndef QDetailWidgetManager_h__
#define QDetailWidgetManager_h__

#include "QMetaType"
#include "QHash"
#include <functional>
#include "Customization/Item/QDetailWidgetPropertyItem.h"
#include "Instance/QInstanceDetail.h"
#include "QMap"

inline size_t qHash(const QMetaType& key, size_t seed)
{
	return  key.id();
}

class QDetailWidgetManager {
public:
	using PropertyTypeFilter = std::function<bool(QMetaType)>;
	using PropertyItemCreator = std::function<QDetailWidgetPropertyItem* ()>;

	using InstanceTypeFilter = std::function<bool(const QSharedPointer<QInstance>& )>;
	using InstanceDetailCreator = std::function< QInstanceDetail*()>;

	QDetailWidgetManager();

	static QDetailWidgetManager* instance();

	template<typename PropertyItemType>
	void RegisterPropertyItemCreator() {
		QList<QMetaType> typeList = PropertyItemType::SupportedTypes();
		PropertyItemCreator creator = []() {
			return new PropertyItemType();
		};
		for (QMetaType type : typeList) {
			mPropertyItemCreatorMap[type] = creator;
		}
	}

	template<typename PropertyItemType>
	void RegisterPropertyItemFilter() {
		PropertyTypeFilter filter= [](QMetaType inType) {
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

	const QHash<QMetaType,PropertyItemCreator>& GetPropertyItemCreatorMap() const { return mPropertyItemCreatorMap; }
	const QList<QPair<PropertyTypeFilter,PropertyItemCreator>>& GetPropertyItemFilterList() const { return mPropertyItemFilterList; }
	const QList<QPair<InstanceTypeFilter, InstanceDetailCreator>>& GetInstanceDetailFilterList() const { return mInstanceFilterList; }
protected:
	void RegisterBuiltIn();

private:
	QHash<QMetaType, PropertyItemCreator> mPropertyItemCreatorMap;
	QList<QPair<PropertyTypeFilter, PropertyItemCreator>>  mPropertyItemFilterList;
	QList<QPair<InstanceTypeFilter, InstanceDetailCreator>>  mInstanceFilterList;
};

#endif // QDetailWidgetManager_h__
