#ifndef QObjectPropertyHandleImpl_h__QObjectPropertyHandleImpl
#define QObjectPropertyHandleImpl_h__

#include "IPropertyHandleImpl.h"
#include "QVariant"

class QObjectPropertyHandleImpl : public IPropertyHandleImpl {
public:
	QObjectPropertyHandleImpl(QPropertyHandle* InHandle);
protected:
	QPropertyHandle* FindChildHandle(const QString& inSubName) override;
	QWidget* GenerateValueWidget() override;
	void GenerateChildrenRow(QRowLayoutBuilder* Builder)  override;
	QPropertyHandle* CreateChildHandle(const QString& inSubName) override;
	void RefreshObjectPtr();
private:
	QVariant mObjectHolder;
	void* mObjectPtr = nullptr;
	QObject* mOwnerObject = nullptr;
	const QMetaObject* mMetaObject = nullptr;
	bool bIsSharedPointer = false;
	bool bIsPointer = false;
};

#endif // QObjectPropertyHandleImpl_h__
