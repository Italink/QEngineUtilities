#ifndef QObjectDetailBuilder_h__
#define QObjectDetailBuilder_h__

#include "QObject"
#include "QObjectEx.h"

class QTreeWidgetItem; 
class QDetailWidget;

class QObjectDetailBuilder {
public:
	QObjectDetailBuilder(QObject* inObject, QDetailWidget* inWidget );

	void BuildDefault();

	void AddNewProperty(QMetaProperty inProperty);

	QString GetPropertyCategoryName(QString inPropertyName);

	QJsonObject GetPropertyMetaData(QString inPropertyName);
protected:
	void ReadObjectMetaData();
public:
	QObject* mObject = nullptr;
	QDetailWidget* mWidget = nullptr;
	QObjectMetaData mMetaData;
};

#endif // QObjectDetailBuilder_h__
