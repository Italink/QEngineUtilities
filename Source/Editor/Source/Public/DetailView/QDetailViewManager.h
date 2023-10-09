#ifndef QDetailWidgetManager_h__
#define QDetailWidgetManager_h__

#include <functional>
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"

class QENGINEEDITOR_API QDetailViewManager {
public:
	using CustomClassLayoutCreator = std::function<QSharedPointer<IDetailCustomization>()>;
	using CustomPropertyTypeLayoutCreator = std::function<QSharedPointer<IPropertyTypeCustomization>()>;
	using CustomPropertyValueWidgetCreator = std::function<QWidget*(QPropertyHandle*)>;

	static QDetailViewManager* Instance();
	template<typename IDetailCustomizationType>
	void registerCustomClassLayout(const QMetaObject* InMetaObject)
	{
		mCustomClassLayoutMap.insert(InMetaObject, []() {
			return QSharedPointer<IDetailCustomizationType>::create();
		});
	}
	void unregisterCustomClassLayout(const QMetaObject* InMetaObject);

	template<typename MetaType, typename IPropertyTypeCustomizationType>
	void registerCustomPropertyTypeLayout(){
		mCustomPropertyTypeLayoutMap.insert(QMetaType::fromType<MetaType>(), []() {
			return QSharedPointer<IPropertyTypeCustomizationType>::create();
		});
	}
	void unregisterCustomClassLayout(const QMetaType& InMetaType);


	void registerCustomPropertyValueWidgetCreator(const QMetaType& InMetaType, CustomPropertyValueWidgetCreator Creator);
	void unregisterCustomPropertyValueWidgeCreator(const QMetaType& InMetaType);

	QSharedPointer<IDetailCustomization> getCustomDetailLayout(const QMetaObject* InMetaObject);
	QSharedPointer<IPropertyTypeCustomization> getCustomPropertyType(const QMetaType& InMetaType);
	QWidget* getCustomPropertyValueWidget(QPropertyHandle* InHandler);
protected:
	QDetailViewManager();
	void registerBuiltIn();
private:
	QHash<const QMetaObject*, CustomClassLayoutCreator> mCustomClassLayoutMap;
	QHash<QMetaType, CustomPropertyTypeLayoutCreator  > mCustomPropertyTypeLayoutMap;
	QHash<QMetaType, CustomPropertyValueWidgetCreator > mCustomPropertyValueWidgetMap;
};


#endif // QDetailWidgetManager_h__
