#ifndef QAssociativePropertyHandle_h__
#define QAssociativePropertyHandle_h__

#include "QMetaContainer"
#include "IPropertyHandleImpl.h"

class QAssociativePropertyHandleImpl: public IPropertyHandleImpl {
public:
	QAssociativePropertyHandleImpl(QPropertyHandle* InHandle);

	void AppendItem(QString inKey, QVariant inValue);
	bool RenameItem(QString inSrc, QString inDst);
	void RemoveItem(QString inKey);
protected:
	void GenerateChildrenRow(QRowLayoutBuilder* Builder)  override;
	QWidget* GenerateValueWidget() override;
	QPropertyHandle* CreateChildHandle(const QString& inSubName) override;
private:
	QMetaAssociation mMetaAssociation;
};


#endif // QAssociativePropertyHandle_h__

