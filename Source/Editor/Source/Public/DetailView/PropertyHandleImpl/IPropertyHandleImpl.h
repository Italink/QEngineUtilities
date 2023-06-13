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
	virtual QPropertyHandle* FindChildHandle(const QString& inSubName);
	virtual QPropertyHandle* CreateChildHandle(const QString& inSubName);
	virtual QWidget* GenerateNameWidget();
	virtual QWidget* GenerateValueWidget();
	virtual void GenerateChildrenRow(QRowLayoutBuilder* Builder) {}
protected:
	QPropertyHandle* mHandle;
};

#endif // IPropertyHandleImpl_h__
