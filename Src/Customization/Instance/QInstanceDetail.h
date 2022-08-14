#ifndef QInstanceDetail_h__
#define QInstanceDetail_h__

#include "QObject"

#include "Core/QMetaDataDefine.h"
#include "Core/QInstance.h"
#include "Core/QPropertyHandler.h"

class QTreeWidgetItem; 
class QDetailWidgetItem;
class QDetailTreeWidget;
class QDetailWidgetCategoryItem;

class QInstanceDetail: public QObject{
	Q_OBJECT
public:
	static QInstanceDetail* FindOrCreate(QSharedPointer<QInstance> inInstance, QDetailTreeWidget* inWidget);
	static QInstanceDetail* FindOrCreate(QSharedPointer<QInstance> inInstance, QDetailWidgetItem* inRoot);

	virtual void Initialize(QSharedPointer<QInstance> inInstance, QDetailTreeWidget* inWidget);
	virtual void Initialize(QSharedPointer<QInstance> inInstance, QDetailWidgetItem* inRoot);
	virtual void Build();
protected:
	QDetailWidgetCategoryItem* FindOrAddCategory(QString inName);
	void AddProperty(QPropertyHandler* inPropertyHandler);
private:
	void PreBuildCategory();
protected:
	QSharedPointer<QInstance> mInstance;
	QDetailTreeWidget* mWidget = nullptr;
	QDetailWidgetItem* mRoot = nullptr;
};
#endif // QInstanceDetail_h__
