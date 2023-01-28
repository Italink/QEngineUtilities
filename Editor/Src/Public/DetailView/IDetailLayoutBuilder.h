#ifndef IDETAILLAYOUTBUILDER_H
#define IDETAILLAYOUTBUILDER_H

#include <QSharedPointer>
#include <QObject>
#include <QHash>

class IPropertyTypeCustomization;
class IDetailCustomization;
class QPropertyHandle;
class QDetailView;
class QDetailViewRow;

class IDetailLayoutBuilder {
public:
	struct ObjectContext {
		void* ObjectPtr = nullptr;
		const QMetaObject* MetaObject;

		QObject* OwnerObject = nullptr;
		QString PrePath;
	};

	IDetailLayoutBuilder* AddRowByWholeContent(QWidget* InContent);
	IDetailLayoutBuilder* AddRowByNameValueWidget(QWidget* InName, QWidget* InValue);

	void AddProperty(QPropertyHandle* InPropertyHandle);
	void AddObject(QObject* InObject, QString InPrePath = QString(), bool HideHeader = true);
	void AddObject(IDetailLayoutBuilder::ObjectContext Context, bool HideHeader = true);

	IDetailLayoutBuilder* FindOrAddCategory(const QString& InName);
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
