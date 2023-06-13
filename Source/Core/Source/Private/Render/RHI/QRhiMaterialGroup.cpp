#include "Render/RHI/QRhiMaterialGroup.h"

QRhiMaterialGroup::QRhiMaterialGroup(QList<QSharedPointer<QMaterial>> inMaterials) {
	for (auto& material : inMaterials) {
		addMaterial(material);
	}
}

QRhiMaterialGroup::QRhiMaterialGroup(QSharedPointer<QMaterial> inMaterial) {
	addMaterial(inMaterial);
}

void QRhiMaterialGroup::addMaterial(QSharedPointer<QMaterial> inMaterial) {
	QSharedPointer<QRhiMaterialDesc> desc = QSharedPointer<QRhiMaterialDesc>::create();
	desc->uniformBlock = QSharedPointer<QRhiUniformBlock>::create();
	desc->uniformBlock->setObjectName("Material");
	for (const auto& pair : inMaterial->mProperties.asKeyValueRange()) {
		if (pair.second.metaType() == QMetaType::fromType<float>()) {
			desc->uniformBlock->addParam(pair.first, pair.second.value<float>());
		}
		else if (pair.second.metaType() == QMetaType::fromType<QVector2D>()) {
			desc->uniformBlock->addParam(pair.first, pair.second.value<QVector2D>());
		}
		else if (pair.second.metaType() == QMetaType::fromType<QVector3D>()) {
			desc->uniformBlock->addParam(pair.first, pair.second.value<QVector3D>());
		}
		else if (pair.second.metaType() == QMetaType::fromType<QVector3D>()) {
			desc->uniformBlock->addParam(pair.first, pair.second.value<QVector3D>());
		}
		else if (pair.second.metaType() == QMetaType::fromType<QVector4D>()) {
			desc->uniformBlock->addParam(pair.first, pair.second.value<QVector4D>());
		}
		else if (pair.second.metaType() == QMetaType::fromType<QColor>()) {
			desc->uniformBlock->addParam(pair.first, pair.second.value<QColor>());
		}
		else if (pair.second.metaType() == QMetaType::fromType<QImage>()) {
			QSharedPointer<QRhiTextureDesc> textureDesc = QSharedPointer<QRhiTextureDesc>::create();
			QImage image = pair.second.value<QImage>().convertedTo(QImage::Format_RGBA8888);
			textureDesc->Name = pair.first;
			textureDesc->ImageCache = image;
			textureDesc->Size = image.size();
			textureDesc->Format = QRhiTexture::RGBA8;
			textureDesc->GlslTypeName = "sampler2D";
			textureDesc->MagFilter = QRhiSampler::Filter::Linear;
			textureDesc->MinFilter = QRhiSampler::Filter::Nearest;
			textureDesc->MipmapMode = QRhiSampler::Filter::Linear;
			textureDesc->AddressU = QRhiSampler::AddressMode::Mirror;
			textureDesc->AddressV = QRhiSampler::AddressMode::Mirror;
			textureDesc->AddressW = QRhiSampler::AddressMode::Mirror;
			textureDesc->UploadDesc = QRhiTextureUploadDescription({
				QRhiTextureUploadEntry(0,0,QRhiTextureSubresourceUploadDescription(textureDesc->ImageCache))
			});
			desc->textureDescs << textureDesc;
		}
	}
	mInfoMap[inMaterial] = desc;
	mDescList << desc;
}

QSharedPointer<QRhiMaterialDesc> QRhiMaterialGroup::getMaterialDesc(int inIndex) {
	return mDescList.value(inIndex);
}

QString QRhiMaterialDesc::getOrCreateBaseColorExpression() {
	if (auto param = getTexture("BaseColor")) {
		return "texture(uBaseColor,vUV)";
	}
	else if (auto param = getTexture("Diffuse")) {
		return "texture(uDiffuse,vUV)";
	}
	else if (auto param = uniformBlock->getParamDesc("BaseColor")) {
		if(param->mValue.metaType() == QMetaType::fromType<QVector4D>()|| param->mValue.metaType() == QMetaType::fromType<QColor>() || param->mValue.metaType() == QMetaType::fromType<QColor4D>())
			return "Material.BaseColor";
	}
	else if (auto param = uniformBlock->getParamDesc("Diffuse")) {
		if (param->mValue.metaType() == QMetaType::fromType<QVector4D>() || param->mValue.metaType() == QMetaType::fromType<QColor>() || param->mValue.metaType() == QMetaType::fromType<QColor4D>())
			return "Material.Diffuse";
	}
	uniformBlock->addParam("BaseColor", QColor(255, 255, 255));
	return "Material.BaseColor";
}

QString QRhiMaterialDesc::getOrCreateSpecularExpression() {
	if (auto param = getTexture("Specular")) {
		return "texture(uSpecular,vUV).r";
	}
	if (auto param = uniformBlock->getParamDesc("Specular")) {
		if (param->mValue.metaType() == QMetaType::fromType<float>())
			return "Material.Specular";
	}
	uniformBlock->addParam("Specular", 0.5f);
	return "Material.Specular";
}

QString QRhiMaterialDesc::getOrCreateMetallicExpression() {
	if (auto param = getTexture("Metallic")) {
		return "texture(uMetallic,vUV).r";
	}
	if (auto param = uniformBlock->getParamDesc("Metallic")) {
		if (param->mValue.metaType() == QMetaType::fromType<float>())
			return "Material.Metallic";
	}
	uniformBlock->addParam("Metallic", 0.5f);
	return "Material.Metallic";
}

QString QRhiMaterialDesc::getOrCreateRoughnessExpression() {
	if (auto param = getTexture("Roughness")) {
		if(param->ImageCache.pixelFormat().channelCount() == 1)
			return "texture(uRoughness,vUV).r";
		else
			return "texture(uRoughness,vUV).g";
	}
	if (auto param = uniformBlock->getParamDesc("Roughness")) {
		if (param->mValue.metaType() == QMetaType::fromType<float>())
			return "Material.Roughness";
	}
	uniformBlock->addParam("Roughness", 0.5f);
	return "Material.Roughness";
}

QString QRhiMaterialDesc::getNormalExpression() {
	if (auto param = getTexture("Normal")) {
		return "texture(uNormal,vUV).rgb";
	}
	return "vec3(1)";
}

QSharedPointer<QRhiTextureDesc> QRhiMaterialDesc::getTexture(const QString& inName) {
	for (auto desc : textureDescs) {
		if (desc->Name == inName)
			return desc;
	}
	return QSharedPointer<QRhiTextureDesc>();
}
