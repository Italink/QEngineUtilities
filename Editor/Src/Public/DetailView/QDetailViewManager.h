#ifndef QDetailWidgetManager_h__
#define QDetailWidgetManager_h__

#include <functional>
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "Utils/QMetaData.h"

class QDetailViewManager {
public:
	using CustomClassLayoutCreator = std::function<QSharedPointer<IDetailCustomization>()>;
	using CustomPropertyTypeLayoutCreator = std::function<QSharedPointer<IPropertyTypeCustomization>()>;
	using CustomPropertyValueWidgetCreator = std::function<QWidget*(QPropertyHandle*)>;

	static QDetailViewManager* Instance();
	template<typename IDetailCustomizationType>
	void RegisterCustomClassLayout(const QMetaObject* InMetaObject)
	{
		mCustomClassLayoutMap.insert(InMetaObject, []() {
			return QSharedPointer<IDetailCustomizationType>::create();
		});
	}
	void UnregisterCustomClassLayout(const QMetaObject* InMetaObject);

	template<typename MetaType, typename IPropertyTypeCustomizationType>
	void RegisterCustomPropertyTypeLayout(){
		mCustomPropertyTypeLayoutMap.insert(QMetaType::fromType<MetaType>(), []() {
			return QSharedPointer<IPropertyTypeCustomizationType>::create();
		});
	}
	void UnregisterCustomClassLayout(const QMetaType& InMetaType);


	void RegisterCustomPropertyValueWidgetCreator(const QMetaType& InMetaType, CustomPropertyValueWidgetCreator Creator);
	void UnregisterCustomPropertyValueWidgeCreator(const QMetaType& InMetaType);

	QSharedPointer<IDetailCustomization> GetCustomDetailLayout(const QMetaObject* InMetaObject);
	QSharedPointer<IPropertyTypeCustomization> GetCustomPropertyType(const QMetaType& InMetaType);
	QWidget* GetCustomPropertyValueWidget(QPropertyHandle* InHandler);
	QMetaData* GetClassMetaData(const IDetailLayoutBuilder::ObjectContext& Context);
protected:
	QDetailViewManager();
	void RegisterBuiltIn();
private:
	QHash<const QMetaObject*, CustomClassLayoutCreator> mCustomClassLayoutMap;
	QHash<QMetaType, CustomPropertyTypeLayoutCreator  > mCustomPropertyTypeLayoutMap;
	QHash<QMetaType, CustomPropertyValueWidgetCreator > mCustomPropertyValueWidgetMap;
	QHash<const QMetaObject*, QSharedPointer<QMetaData>> mClassMetaData;
};


#endif // QDetailWidgetManager_h__
