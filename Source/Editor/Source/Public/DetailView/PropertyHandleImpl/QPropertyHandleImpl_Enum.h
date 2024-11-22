#ifndef QPropertyHandleImpl_Enum_h__
#define QPropertyHandleImpl_Enum_h__

#include "IPropertyHandleImpl.h"

class QENGINEEDITOR_API QPropertyHandleImpl_Enum: public IPropertyHandleImpl {
public:
	QPropertyHandleImpl_Enum(QPropertyHandle* inHandle);
protected:
	QQuickItem* createValueEditor(QQuickItem* inParent)override;
private:
	QHash<QString, int> mNameToValueMap;
	QList<QString> mKeys;
};


#endif // QPropertyHandleImpl_Enum_h__
