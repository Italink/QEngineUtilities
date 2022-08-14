#ifndef QDetailWidgetPropertyInstanceItem_h__
#define QDetailWidgetPropertyInstanceItem_h__

#include "QDetailWidgetPropertyItem.h"
class QInstance;

class QDetailWidgetPropertyInstanceItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(TypeId inID);
	virtual void SetHandler(QPropertyHandler* inHandler) override;
	virtual void ResetValue() override;
protected:
	void RecreateInstance();
	void RecreateChildren();
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	const QMetaObject* mMetaObject = nullptr;
	QSharedPointer<QInstance> mInstance;
	QVariant mInstanceVar;
	bool bIsSharedPointer = false;
	bool bIsPointer = false;
};

#endif // QDetailWidgetPropertyInstanceItem_h__


