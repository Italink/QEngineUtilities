#ifndef QEnumPropertyHandleImpl_h__
#define QEnumPropertyHandleImpl_h__

#include "IPropertyHandleImpl.h"

class QEnumPropertyHandleImpl: public IPropertyHandleImpl {
public:
	QEnumPropertyHandleImpl(QPropertyHandle* InHandle);
protected:
	QWidget* GenerateValueWidget() override;
private:
	QHash<QString, int> mNameToValueMap;
};


#endif // QEnumPropertyHandleImpl_h__
