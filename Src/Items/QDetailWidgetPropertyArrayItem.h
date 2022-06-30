#ifndef QDetailWidgetPropertyArrayItem_h__
#define QDetailWidgetPropertyArrayItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertyArrayItem : public QDetailWidgetPropertyItem {
public:
	QDetailWidgetPropertyArrayItem();

	static bool FilterType(TypeId inID);
	virtual void SetHandler(QPropertyHandler* inHandler) override;
	void ResetValue() override;
protected:
	void FindOrCreateChildItem(int index);
	void RecreateChildren();
	void CreateNewItem();
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	QPropertyHandler::TypeId mValueTypeId = 0;
};

#endif // QDetailWidgetPropertyArrayItem_h__


