#ifndef QQuickDetailsViewManager_h__
#define QQuickDetailsViewManager_h__

#include <QObject>
#include <QHash>
#include <functional>
#include <QMetaType>
#include <QQmlEngine>
#include <QQuickItem>
#include "IDetailCustomization.h"
#include "IPropertyTypeCustomization.h"
#include "QEngineEditorAPI.h"

class QPropertyHandle;

class QENGINEEDITOR_API QQuickDetailsViewManager : public QObject{
public:
	using CustomClassLayoutCreator = std::function<QSharedPointer<IDetailCustomization>()>;
	using CustomPropertyTypeLayoutCreator = std::function<QSharedPointer<IPropertyTypeCustomization>()>;
	using CustomPropertyValueWidgetCreator = std::function<QQuickItem* (QPropertyHandle*, QQuickItem*)>;

	static QQuickDetailsViewManager* Get();

	template<typename IDetailCustomizationType>
	void registerCustomClassLayout(const QMetaObject* InMetaObject)
	{
		mCustomClassLayoutMap.insert(InMetaObject, []() {
			return QSharedPointer<IDetailCustomizationType>::create();
		});
	}
	void unregisterCustomClassLayout(const QMetaObject* InMetaObject);

	template<typename MetaType, typename IPropertyTypeCustomizationType>
	void registerCustomPropertyTypeLayout() {
		mCustomPropertyTypeLayoutMap.insert(QMetaType::fromType<MetaType>(), []() {
			return QSharedPointer<IPropertyTypeCustomizationType>::create();
			});
	}
	void unregisterCustomPropertyTypeLayout(const QMetaType& InMetaType);

	void registerCustomPropertyValueEditorCreator(const QMetaType& inMetaType, CustomPropertyValueWidgetCreator Creator);
	void unregisterCustomPropertyValueEditorCreator(const QMetaType& inMetaType);

	QQuickItem* createValueEditor(QPropertyHandle* inHandle, QQuickItem* parent);
	QSharedPointer<IDetailCustomization> getCustomDetailLayout(const QMetaObject* InMetaObject);
	QSharedPointer<IPropertyTypeCustomization> getCustomPropertyType(const QMetaType& InMetaType);
protected:
	QQuickDetailsViewManager();
private:
	QHash<const QMetaObject*, CustomClassLayoutCreator> mCustomClassLayoutMap;
	QHash<QMetaType, CustomPropertyTypeLayoutCreator  > mCustomPropertyTypeLayoutMap;
	QHash<QMetaType, CustomPropertyValueWidgetCreator> mPropertyValueEditorCreatorMap;
};

#endif // QQuickDetailsViewManager_h__
