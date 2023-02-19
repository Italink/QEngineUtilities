#include "DetailCustomization_QRhiGraphicsPipelineBuilder.h"
#include <QMetaProperty>
#include "DetailView/QDetailLayoutBuilder.h"
#include "DetailView/QDetailViewManager.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"

void DetailCustomization_QRhiGraphicsPipelineBuilder::CustomizeDetails(const IDetailLayoutBuilder::ObjectContext& Context, IDetailLayoutBuilder* Builder) {
	QRhiGraphicsPipelineBuilder* pipeline = (QRhiGraphicsPipelineBuilder*) Context.ObjectPtr;
	QMetaData* metaData = QDetailViewManager::Instance()->GetClassMetaData(Context);
	QPropertyHandle* uniformblocksHandle = QPropertyHandle::FindOrCreate(pipeline, "UniformBlocks", metaData->mPropertiesMetaData.value("UniformBlocks"));
	for (auto uniformblock : pipeline->getUniformBlocks().keys()) {
		if(uniformblock != "Transform")
			Builder->AddProperty(uniformblocksHandle->CreateChildHandle(uniformblock));
	}
	for (auto texture : pipeline->getTextures().asKeyValueRange()) {
		QString path = "Textures." + texture.first;
		QPropertyHandle* handler = QPropertyHandle::FindOrCreate(
			pipeline,
			QMetaType::fromType<QImage>(),
			path,
			[textureInfo = texture.second]() {
			return textureInfo->Image;
		},
		[textureInfo = texture.second](QVariant var) {
			textureInfo->Image = var.value<QImage>();
		}
		, metaData->mPropertiesMetaData.value(texture.first)
		);
		Builder->AddProperty(handler);
	}
}
