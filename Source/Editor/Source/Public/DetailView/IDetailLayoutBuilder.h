#ifndef IDETAILLAYOUTBUILDER_H
#define IDETAILLAYOUTBUILDER_H

#include <QSharedPointer>
#include <QObject>
#include <QHash>
#include "QEngineEditorAPI.h"

class IPropertyTypeCustomization;
class IDetailCustomization;
class QPropertyHandle;
class QDetailView;
class QDetailViewRow;

class QENGINEEDITOR_API IDetailLayoutBuilder {
public:
	struct ObjectContext {
		void* ObjectPtr = nullptr;
		const QMetaObject* MetaObject;
		QObject* OwnerObject = nullptr;
		QString PrePath;
	};

	void setPage(QWidget* InPage);
	IDetailLayoutBuilder* addRowByWholeContent(QWidget* InContent);
	IDetailLayoutBuilder* addRowByNameValueWidget(QWidget* InName, QWidget* InValue);
	IDetailLayoutBuilder* addRowByNameValueWidget(const QString& inName, QWidget* InValue);

	void addProperty(QPropertyHandle* InPropertyHandle);
	void addObject(QObject* InObject, QString InPrePath = QString());
	void addObject(IDetailLayoutBuilder::ObjectContext Context);

	bool isChildrenVisible() const;
	bool isIgnoredType(const QMetaObject* inMetaObj);

	IDetailLayoutBuilder* findOrAddCategory(const QString& InName);

	virtual QDetailViewRow* row() { return nullptr; }
protected:
	IDetailLayoutBuilder(QDetailView* InDetailView);
	virtual QDetailViewRow* newChildRow() = 0;
protected:
	QDetailView* mDetailView = nullptr;
	QList<QSharedPointer<IDetailLayoutBuilder>> mChildren;
	QHash<void*, QSharedPointer<IDetailCustomization>> mClassCustomizationMap;
	QHash<QPropertyHandle*, QSharedPointer<IPropertyTypeCustomization>> mPropertyTypeCustomizationMap;
	QHash<QString, QSharedPointer<IDetailLayoutBuilder>> mCategoryMap;
};

#endif // IDETAILLAYOUTBUILDER_H
