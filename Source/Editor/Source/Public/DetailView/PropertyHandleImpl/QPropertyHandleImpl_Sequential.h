#ifndef QSequentialPropertyHandle_h__
#define QSequentialPropertyHandle_h__

#include "QMetaContainer"
#include "IPropertyHandleImpl.h"

class QENGINEEDITOR_API QPropertyHandleImpl_Sequential: public IPropertyHandleImpl
{
public:
	QPropertyHandleImpl_Sequential(QPropertyHandle* inHandle);

	int itemCount();
	void appendItem(QVariant InVar);
	void moveItem(int InSrcIndex, int InDstIndex);
	void removeItem(int InIndex);

protected:
	QQuickItem* createValueEditor(QQuickItem* inParent)override;
	QPropertyHandle* createChildHandle(const QString& inSubName) override;
private:
	QMetaSequence mMetaSequence;
};

#endif // QSequentialPropertyHandle_h__
