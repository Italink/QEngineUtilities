#include "QMaterial.h"
#include "assimp/vector3.h"
#include "assimp/matrix4x4.h"
#include "QMatrix4x4"
#include "assimp/scene.h"
#include "QDir"
#include "QImage"


QVector<QSharedPointer<QMaterial>> QMaterial::CreateFromScene(const aiScene* scene, QString modelPath) {
	QDir modelDir = QFileInfo(modelPath).dir();
	QVector<QSharedPointer<QMaterial>> materialList;
	static QStringList TextureNameMap = { "None","Diffuse","Specular","Ambient","Emissive","Height","Normal","Shininess","Opacity","Displacement","LightMap","Reflection",
		"BaseColor","NormalCamera","EmissionColor","Metallic","Roughness","AmbientOcclusion",
		"Unknown","Sheen","ClearCoat","Transmission" };

	for (uint i = 0; i < scene->mNumMaterials; i++) {
		QSharedPointer<QMaterial> material = QSharedPointer<QMaterial>::create();
		aiMaterial* rawMaterial = scene->mMaterials[i];
		for (int i = aiTextureType_DIFFUSE; i < AI_TEXTURE_TYPE_MAX; i++) {
			if (i == aiTextureType_UNKNOWN)
				continue;
			int count = rawMaterial->GetTextureCount(aiTextureType(i));
			for (int j = 0; j < count; j++) {
				aiString path;
				rawMaterial->GetTexture(aiTextureType(i), j, &path);
				QString name = rawMaterial->GetName().C_Str();
				if (name.startsWith('/')) {
					QString newPath = modelDir.filePath(name.mid(1, name.lastIndexOf('/') - 1));
					modelDir.setPath(newPath);
				}
				QString realPath = modelDir.filePath(path.C_Str());
				QImage image;
				if (QFile::exists(realPath)) {
					image.load(realPath);
				}
				else {
					const aiTexture* embTexture = scene->GetEmbeddedTexture(path.C_Str());
					if (embTexture != nullptr) {
						if (embTexture->mHeight == 0) {
							image.loadFromData((uchar*)embTexture->pcData, embTexture->mWidth, embTexture->achFormatHint);
						}
						else {
							image = QImage((uchar*)embTexture->pcData, embTexture->mWidth, embTexture->mHeight, QImage::Format_ARGB32);
						}
					}
				}
				QString slotName = TextureNameMap[i];
				if (j != 0) {
					slotName += QString::number(j);
				}
				if (!image.isNull()) {
					material->mProperties[slotName] = image;
				}
			}
		}
		materialList << material;
	}
	return materialList;
}
