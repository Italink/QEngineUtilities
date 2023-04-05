#ifndef QRhiMaterialGroup_h__
#define QRhiMaterialGroup_h__

#include <QObject>
#include "Asset/QMaterial.h"
#include "Render/RHI/QRhiUniformBlock.h"

struct QRhiTextureDesc {
	QString Name;
	QImage ImageCache;
	QSize Size;
	QRhiTexture::Format Format;
	QRhiTexture::Flags Flags;
	QString GlslTypeName;
	QRhiTextureUploadDescription UploadDesc;
	QRhiSampler::Filter MagFilter;
	QRhiSampler::Filter MinFilter;
	QRhiSampler::Filter MipmapMode;
	QRhiSampler::AddressMode AddressU;
	QRhiSampler::AddressMode AddressV;
	QRhiSampler::AddressMode AddressW;

	QRhiEx::Signal sigUpdate;
	QScopedPointer<QRhiTexture> Texture;
	QSharedPointer<QRhiSampler> Sampler;
};
Q_DECLARE_METATYPE(QRhiTextureDesc*);

struct QRhiMaterialDesc {
	QString getOrCreateBaseColorExpression();
	QString getOrCreateMetallicExpression();
	QString getOrCreateRoughnessExpression();
	QString getNormalExpression();
	QSharedPointer<QRhiTextureDesc> getTexture(const QString& inName);
	QSharedPointer<QRhiUniformBlock> uniformBlock;
	QList<QSharedPointer<QRhiTextureDesc>> textureDescs;
};

class QRhiMaterialGroup : public QObject {
	Q_OBJECT
public:
	QRhiMaterialGroup(QList<QSharedPointer<QMaterial>> inMaterials);
	QSharedPointer<QRhiMaterialDesc> getMaterialDesc(int inIndex);
protected:
	void addMaterial(QSharedPointer<QMaterial> inMaterial);
private:
	QMap<QSharedPointer<QMaterial>, QSharedPointer<QRhiMaterialDesc>> mInfoMap;
	QList<QSharedPointer<QRhiMaterialDesc>> mDescList;
};

#endif // QRhiMaterialGroup_h__
