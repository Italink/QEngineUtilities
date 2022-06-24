#ifndef QDetailWidgetPropertyMapItem_h__
#define QDetailWidgetPropertyMapItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertyMapItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(TypeId inID);
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
};

#endif // QDetailWidgetPropertyMapItem_h__


