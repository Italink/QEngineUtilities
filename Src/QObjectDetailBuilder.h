#ifndef QObjectDetailBuilder_h__
#define QObjectDetailBuilder_h__

#include "QObject"
#include "QMetaDataDefine.h"
#include "QInstance.h"

class QTreeWidgetItem; 
class QDetailTreeWidget;

class QObjectDetailBuilder {
public:
	QObjectDetailBuilder(QSharedPointer<QInstance> inInstance, QDetailTreeWidget* inWidget );

	void BuildDefault();

	void AddNewProperty(QMetaProperty inProperty);

	QString GetPropertyCategoryName(QString inPropertyName);

	QVariantHash GetPropertyMetaData(QMetaProperty inProperty);
protected:
	void ReadObjectMetaData();
public:
	QSharedPointer<QInstance> mInstance;
	QDetailTreeWidget* mWidget = nullptr;
	QObjectMetaData mMetaData;
};

#endif // QObjectDetailBuilder_h__
