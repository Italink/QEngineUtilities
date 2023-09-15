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

	void SetPage(QWidget* InPage);
	IDetailLayoutBuilder* AddRowByWholeContent(QWidget* InContent);
	IDetailLayoutBuilder* AddRowByNameValueWidget(QWidget* InName, QWidget* InValue);
	IDetailLayoutBuilder* AddRowByNameValueWidget(const QString& inName, QWidget* InValue);

	void AddProperty(QPropertyHandle* InPropertyHandle);
	void AddObject(QObject* InObject, QString InPrePath = QString());
	void AddObject(IDetailLayoutBuilder::ObjectContext Context);

	bool ShowChildren() const;
	bool IsIgnoreMetaObject(const QMetaObject* inMetaObj);

	IDetailLayoutBuilder* FindOrAddCategory(const QString& InName);

	virtual QDetailViewRow* Row() { return nullptr; }
protected:
	IDetailLayoutBuilder(QDetailView* InDetailView);
	virtual QDetailViewRow* NewChildRow() = 0;
protected:
	QDetailView* mDetailView = nullptr;
	QList<QSharedPointer<IDetailLayoutBuilder>> mChildren;
	QHash<void*, QSharedPointer<IDetailCustomization>> mClassCustomizationMap;
	QHash<QPropertyHandle*, QSharedPointer<IPropertyTypeCustomization>> mPropertyTypeCustomizationMap;
	QHash<QString, QSharedPointer<IDetailLayoutBuilder>> mCategoryMap;
};

#endif // IDETAILLAYOUTBUILDER_H
