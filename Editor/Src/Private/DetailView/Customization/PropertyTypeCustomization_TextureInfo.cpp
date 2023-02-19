#include "PropertyTypeCustomization_TextureInfo.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"

void PropertyTypeCustomization_TextureInfo::CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	QRhiGraphicsPipelineBuilder::TextureInfo* textureInfo = PropertyHandle->GetValue().value<QRhiGraphicsPipelineBuilder::TextureInfo*>();
	Builder->AsNameValueWidget(PropertyHandle->GenerateNameWidget(), nullptr);
}
