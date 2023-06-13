#ifndef DetailCustomization_QRhiGraphicsPipelineBuilder_h__
#define DetailCustomization_QRhiGraphicsPipelineBuilder_h__

#include "DetailView/IDetailCustomization.h"

class QRhiGraphicsPipelineBuilder;

class DetailCustomization_QRhiGraphicsPipelineBuilder : public IDetailCustomization {
protected:
	void CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) override;
};

#endif // DetailCustomization_QRhiGraphicsPipelineBuilder_h__
