#ifndef IPropertyHandleImpl_h__
#define IPropertyHandleImpl_h__

#include <QQmlContext>
#include <QQuickItem>
#include <QVariant>
#include <QObject>
#include "QEngineEditorAPI.h"

class QPropertyHandle;

class QENGINEEDITOR_API IPropertyHandleImpl{
	friend class QPropertyHandle;
protected:
	IPropertyHandleImpl(QPropertyHandle* inHandle);
	virtual QPropertyHandle* findChildHandle(const QString& inSubName);
	virtual QPropertyHandle* createChildHandle(const QString& inSubName);
	virtual QQuickItem* createNameEditor(QQuickItem* inParent);
	virtual QQuickItem* createValueEditor(QQuickItem* inParent)= 0;
protected:
	QPropertyHandle* mHandle;
};

#endif // IPropertyHandleImpl_h__
