#ifndef IPROPERTYTYPECUSTOMIZATION_H
#define IPROPERTYTYPECUSTOMIZATION_H

#include <QSharedPointer>
#include "IDetailLayoutBuilder.h"
#include "QPropertyHandle.h"


class QENGINEEDITOR_API IHeaderRowBuilder{
public:
	virtual void setNameValueWidget(QWidget* InName, QWidget* InValue) = 0;
	virtual void setWholeContent(QWidget* InContent) = 0;
};

class IPropertyTypeCustomization :public  QEnableSharedFromThis<IPropertyTypeCustomization>
{
public:
	virtual void customizeHeader(QPropertyHandle* PropertyHandle , IHeaderRowBuilder* Builder) = 0;

	virtual void customizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) {}
};
#endif // IPROPERTYTYPECUSTOMIZATION_H
