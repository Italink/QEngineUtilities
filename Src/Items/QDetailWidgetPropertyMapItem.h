#ifndef QDetailWidgetPropertyMapItem_h__
#define QDetailWidgetPropertyMapItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertyMapItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(TypeId inID);

	virtual void SetHandler(QPropertyHandler* inHandler) override;
	virtual void ResetValue() override;
protected:
	QDetailWidgetPropertyItem* FindItem(QString inKey);
	void FindOrCreateChildItem(QString inKey);
	bool RenameChild(QString inSrc, QString inDst);
	void RecreateChildren();
	void CreateNewItem();
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	QPropertyHandler::TypeId mValueTypeId = 0;
};

#endif // QDetailWidgetPropertyMapItem_h__


