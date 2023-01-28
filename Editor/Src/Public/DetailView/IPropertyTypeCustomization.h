#ifndef IPROPERTYTYPECUSTOMIZATION_H
#define IPROPERTYTYPECUSTOMIZATION_H

#include <QSharedPointer>
#include "IDetailLayoutBuilder.h"
#include "QPropertyHandle.h"


class IHeaderRowBuilder{
public:
	virtual void AsNameValueWidget(QWidget* InContent) = 0;
	virtual void AsWholeContent(QWidget* InName, QWidget* InValue) = 0;
};

class IPropertyTypeCustomization :public  QEnableSharedFromThis<IPropertyTypeCustomization>
{
public:
	virtual void CustomizeHeader(QPropertyHandle* PropertyHandle , IHeaderRowBuilder* Builder) = 0;

	virtual void CustomizeChildren(QPropertyHandle* PropertyHandle, IDetailLayoutBuilder* Builder) = 0;
};
#endif // IPROPERTYTYPECUSTOMIZATION_H
