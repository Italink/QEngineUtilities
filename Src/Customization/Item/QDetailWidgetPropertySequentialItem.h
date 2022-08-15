#ifndef QDetailWidgetPropertySequentialItem_h__
#define QDetailWidgetPropertySequentialItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertySequentialItem : public QDetailWidgetPropertyItem {
public:
	QDetailWidgetPropertySequentialItem();
	static bool FilterType(QMetaType inID);
	virtual void SetHandler(QPropertyHandler* inHandler) override;
	void ResetValue() override;
protected:
	void FindOrCreateChildItem(int index);
	void MoveItem(int inSrcIndex, int inDstIndex);
	void RemoveItem(int inIndex);
	void RecreateChildren();
	void CreateNewItem();
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	QMetaType mValueType;
	int mCount = 0;
};

#endif // QDetailWidgetPropertySequentialItem_h__


