#ifndef QDetailWidgetPropertyEnumItem_h__
#define QDetailWidgetPropertyEnumItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertyEnumItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(QMetaType inID);
	int GetEnumValueByName(QString inName);
	virtual QString GetKeywords() override;
protected:
	virtual QWidget* GenerateValueWidget() override;
private:
	QHash<QString, int> mNameToValueMap;
};

#endif // QDetailWidgetPropertyEnumItem_h__


