#ifndef QPropertyHandleImpl_RawType_h__
#define QPropertyHandleImpl_RawType_h__

#include "IPropertyHandleImpl.h"

class QENGINEEDITOR_API QPropertyHandleImpl_RawType : public IPropertyHandleImpl {
public:
	QPropertyHandleImpl_RawType(QPropertyHandle* inHandle);
protected:
	QQuickItem* createValueEditor(QQuickItem* inParent) override;
};

#endif // QPropertyHandleImpl_RawType_h__
