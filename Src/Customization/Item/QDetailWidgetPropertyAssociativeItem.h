#ifndef QDetailWidgetPropertyAssociativeItem_h__
#define QDetailWidgetPropertyAssociativeItem_h__

#include "QDetailWidgetPropertyItem.h"

class QDetailWidgetPropertyAssociativeItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(QMetaType inID);
	virtual void SetHandler(QPropertyHandler* inHandler) override;
	virtual void ResetValue() override;
protected:
	QDetailWidgetPropertyItem* FindItem(QString inKey);
	void FindOrCreateChildItem(QString inKey);
	bool RenameChild(QString inSrc, QString inDst);
	void RecreateChildren();
	void CreateNewItem();
	void RemoveItem(QString inKey);
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	QMetaType mValueType;
};

#endif // QDetailWidgetPropertyAssociativeItem_h__


