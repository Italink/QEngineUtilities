#ifndef QDetailWidgetPropertyArrayItem_h__
#define QDetailWidgetPropertyArrayItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertyArrayItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(TypeId inID);
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
};

#endif // QDetailWidgetPropertyArrayItem_h__


