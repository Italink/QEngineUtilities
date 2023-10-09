#ifndef IPropertyHandleImpl_h__
#define IPropertyHandleImpl_h__

#include "DetailView/QDetailLayoutBuilder.h"
#include "QEngineEditorAPI.h"

class QENGINEEDITOR_API IPropertyHandleImpl{
	friend class QPropertyHandle;
protected:
	IPropertyHandleImpl(QPropertyHandle* InHandle){
		mHandle = InHandle;
	}
	virtual QPropertyHandle* findChildHandle(const QString& inSubName);
	virtual QPropertyHandle* createChildHandle(const QString& inSubName);
	virtual QWidget* generateNameWidget();
	virtual QWidget* generateValueWidget();
	virtual void generateChildrenRow(QRowLayoutBuilder* Builder) {}
protected:
	QPropertyHandle* mHandle;
};

#endif // IPropertyHandleImpl_h__
