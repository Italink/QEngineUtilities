#ifndef QDETAILCUSTOMIZATION_H
#define QDETAILCUSTOMIZATION_H

#include "DetailView/IDetailCustomization.h"
#include <QHash>

class DetailCustomization_QObject : public IDetailCustomization {
protected:
	void CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context ,IDetailLayoutBuilder* Builder) override;
};

#endif // QDETAILCUSTOMIZATION_H
