#include "DetailCustomizationPlugin.h"
#include <QDebug>
#include "Asset/QStaticMesh.h"
#include "DetailCustomization_QGlslSandboxRenderPass.h"
#include "DetailCustomization_QMediaPlayer.h"
#include "DetailCustomization_QRhiMaterialGroup.h"
#include "DetailCustomization_QRhiUniformBlock.h"
#include "DetailView/QDetailViewManager.h"
#include "PropertyTypeCustomization_QMatrix4x4.h"
#include "PropertyTypeCustomization_QStaticMesh.h"
#include "PropertyTypeCustomization_TextureInfo.h"
#include "QMediaPlayer"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"
#include "Render/RHI/QRhiMaterialGroup.h"
#include "Render/RHI/QRhiUniformBlock.h"
#include "QColor4DButton.hpp"

void DetailCustomizationPlugin::startup() {
	qDebug() << "DetailCustomizationPlugin::startup";
	QDetailViewManager* mgr = QDetailViewManager::Instance();
	mgr->RegisterCustomClassLayout<DetailCustomization_QRhiUniformBlock>(&QRhiUniformBlock::staticMetaObject);
	mgr->RegisterCustomClassLayout<DetailCustomization_QRhiMaterialGroup>(&QRhiMaterialGroup::staticMetaObject);
	//mgr->RegisterCustomClassLayout<DetailCustomization_QGlslSandboxRenderPass>(&QGlslSandboxRenderPass::staticMetaObject);
	mgr->RegisterCustomClassLayout<DetailCustomization_QMediaPlayer>(&QMediaPlayer::staticMetaObject);

	mgr->RegisterCustomPropertyTypeLayout<QRhiTextureDesc*, PropertyTypeCustomization_TextureInfo>();
	mgr->RegisterCustomPropertyTypeLayout<QSharedPointer<QStaticMesh>, PropertyTypeCustomization_QStaticMesh>();
	mgr->RegisterCustomPropertyTypeLayout<QMatrix4x4, PropertyTypeCustomization_QMatrix4x4>();

	mgr->RegisterCustomPropertyValueWidgetCreator(QMetaType::fromType<QColor4D>(), [](QPropertyHandle* InHandler) {
		QColor4DButton* colorButton = new QColor4DButton();
		InHandler->Bind(
			colorButton, 
			&QColor4DButton::AsColorChanged,
			[colorButton]() {
				return QVariant::fromValue<QColor4D>(colorButton->GetColor());
			},
			[colorButton](QVariant var) {
				colorButton->SetColor(var.value<QColor4D>());
			}
		);
		return colorButton;
	});

	qRegisterMetaType<QRhiTextureDesc>();
	qRegisterMetaType<QRhiGraphicsPipelineBuilder*>();
	qRegisterMetaType<IStaticMeshCreator*>();
}

void DetailCustomizationPlugin::shutdown() {
	qDebug() << "DetailCustomizationPlugin::shutdown";
	QDetailViewManager* mgr = QDetailViewManager::Instance();
	mgr->UnregisterCustomClassLayout(&QRhiUniformBlock::staticMetaObject);
	mgr->UnregisterCustomClassLayout(&QRhiMaterialGroup::staticMetaObject);
	//mgr->UnregisterCustomClassLayout(&QGlslSandboxRenderPass::staticMetaObject);
	mgr->UnregisterCustomClassLayout(&QMediaPlayer::staticMetaObject);

	mgr->UnregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QRhiTextureDesc*>());
	mgr->UnregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QSharedPointer<QStaticMesh>>());
	mgr->UnregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QMatrix4x4>());
	mgr->UnregisterCustomPropertyValueWidgeCreator(QMetaType::fromType<QColor4D>());
}

QENGINE_IMPLEMENT_PLUGIN(DetailCustomizationPlugin, DetailCustomization)
