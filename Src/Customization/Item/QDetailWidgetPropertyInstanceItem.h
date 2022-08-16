#ifndef QDetailWidgetPropertyInstanceItem_h__
#define QDetailWidgetPropertyInstanceItem_h__

#include "QDetailWidgetPropertyItem.h"

class QInstance;
class QComboBox;

class QDetailWidgetPropertyInstanceItem : public QDetailWidgetPropertyItem {
public:
	static bool FilterType(QMetaType inID);
	virtual void SetHandler(QPropertyHandler* inHandler) override;
	virtual void ResetValue() override;
protected:
	void RecreateInstance();
	void RecreateChildren();
	void ReflushInstanceComboBox();
protected:
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	const QMetaObject* mMetaObject = nullptr;
	QSharedPointer<QInstance> mInstance;
	QVariant mInstanceVar;
	bool bIsSharedPointer = false;
	bool bIsPointer = false;

	QComboBox* mInstanceComboBox = nullptr;
};

#endif // QDetailWidgetPropertyInstanceItem_h__


