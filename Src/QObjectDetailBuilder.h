#ifndef QObjectDetailBuilder_h__
#define QObjectDetailBuilder_h__

#include "QObject"
#include "QMetaDataDefine.h"
#include "BindingLayer/BindingLayerDefine.h"

class QTreeWidgetItem; 
class QDetailTreeWidget;

class QObjectDetailBuilder {
public:
	QObjectDetailBuilder(BindingLayer::Instance inObject, QDetailTreeWidget* inWidget );

	void BuildDefault();

	void AddNewProperty(QMetaProperty inProperty);

	QString GetPropertyCategoryName(QString inPropertyName);

	QJsonObject GetPropertyMetaData(QMetaProperty inProperty);
protected:
	void ReadObjectMetaData();
public:
	BindingLayer::Instance mObject = nullptr;
	QDetailTreeWidget* mWidget = nullptr;
	QObjectMetaData mMetaData;
};

#endif // QObjectDetailBuilder_h__
