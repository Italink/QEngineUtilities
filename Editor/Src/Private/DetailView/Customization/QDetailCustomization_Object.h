#ifndef QDETAILCUSTOMIZATION_H
#define QDETAILCUSTOMIZATION_H

#include "DetailView/IDetailCustomization.h"
#include <QHash>

class IPropertyTypeCustomization;
class QPropertyHandle;



class QDetailCustomization_Object : public IDetailCustomization {
protected:
	void CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context ,IDetailLayoutBuilder* Builder) override;
protected:
	QHash<QPropertyHandle*, QSharedPointer<IPropertyTypeCustomization>> mCustomizationMap;
};

#endif // QDETAILCUSTOMIZATION_H
