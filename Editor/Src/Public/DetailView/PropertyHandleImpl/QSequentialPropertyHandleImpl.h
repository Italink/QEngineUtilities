#ifndef QSequentialPropertyHandle_h__
#define QSequentialPropertyHandle_h__

#include "QMetaContainer"
#include "IPropertyHandleImpl.h"
#include "DetailView/QDetailLayoutBuilder.h"

class QSequentialPropertyHandleImpl: public IPropertyHandleImpl
{
public:
	QSequentialPropertyHandleImpl(QPropertyHandle* InHandle);

	int ItemCount();
	void AppendItem(QVariant InVar);
	void MoveItem(int InSrcIndex, int InDstIndex);
	void RemoveItem(int InIndex);
protected:
	void GenerateChildrenRow(QRowLayoutBuilder* Builder)  override;
	QWidget* GenerateValueWidget() override;
	QPropertyHandle* CreateChildHandle(const QString& inSubName) override;
private:
	QMetaSequence mMetaSequence;
};

#endif // QSequentialPropertyHandle_h__
