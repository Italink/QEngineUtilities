#ifndef QObjectPropertyHandleImpl_h__QObjectPropertyHandleImpl
#define QObjectPropertyHandleImpl_h__

#include "IPropertyHandleImpl.h"
#include "QVariant"

class QObjectPropertyHandleImpl : public IPropertyHandleImpl {
public:
	QObjectPropertyHandleImpl(QPropertyHandle* InHandle);
protected:
	QPropertyHandle* findChildHandle(const QString& inSubName) override;
	QWidget* generateValueWidget() override;
	void generateChildrenRow(QRowLayoutBuilder* Builder)  override;
	QPropertyHandle* createChildHandle(const QString& inSubName) override;
	void refreshObjectPtr();
private:
	QVariant mObjectHolder;
	void* mObjectPtr = nullptr;
	QObject* mOwnerObject = nullptr;
	const QMetaObject* mMetaObject = nullptr;
	bool bIsSharedPointer = false;
	bool bIsPointer = false;
};

#endif // QObjectPropertyHandleImpl_h__
