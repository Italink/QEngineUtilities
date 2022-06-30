#ifndef QDetailWidgetCategoryItem_h__
#define QDetailWidgetCategoryItem_h__

#include "QDetailWidgetItem.h"

class QDetailWidgetCategoryItem :public QDetailWidgetItem {
public:
	QDetailWidgetCategoryItem(QString inCategoryName);

	virtual QString GetKeywords() override;

	virtual void BuildContentAndChildren() override;

	virtual void BuildMenu(QMenu& inMenu) override;

	QString GetCategoryName() const;

	void SetCategoryName(QString val);

private:
	QString mCategoryName;
};


#endif // QDetailWidgetCategoryItem_h__
