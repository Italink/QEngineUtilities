#include "PropertyTypeCustomization_TextureInfo.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"

void PropertyTypeCustomization_TextureInfo::CustomizeHeader(QPropertyHandle* PropertyHandle, IHeaderRowBuilder* Builder) {
	QRhiTextureDesc* textureInfo = PropertyHandle->GetValue().value<QRhiTextureDesc*>();
	Builder->AsNameValueWidget(PropertyHandle->GenerateNameWidget(), nullptr);
}