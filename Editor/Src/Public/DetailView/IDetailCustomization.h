#ifndef IDETAILCUSTOMIZATION_H
#define IDETAILCUSTOMIZATION_H

#include <QSharedPointer>
#include "IDetailLayoutBuilder.h"

class IDetailCustomization :public  QEnableSharedFromThis<IDetailCustomization> {
public:
	virtual ~IDetailCustomization(){}

	virtual void CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) = 0;
};

#endif // IDETAILCUSTOMIZATION_H
