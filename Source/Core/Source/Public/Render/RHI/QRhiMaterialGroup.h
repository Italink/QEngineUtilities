#ifndef QRhiMaterialGroup_h__
#define QRhiMaterialGroup_h__

#include <QObject>
#include "Asset/QMaterial.h"
#include "Render/RHI/QRhiUniformBlock.h"
#include "QEngineCoreAPI.h"

struct QENGINECORE_API QRhiTextureDesc {
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

	QRhiSignal sigUpdate;
	QScopedPointer<QRhiTexture> Texture;
	QSharedPointer<QRhiSampler> Sampler;
};
Q_DECLARE_METATYPE(QRhiTextureDesc*);

struct QENGINECORE_API QRhiMaterialDesc {
	QString getOrCreateBaseColorExpression();
	QString getOrCreateSpecularExpression();
	QString getOrCreateMetallicExpression();
	QString getOrCreateRoughnessExpression();
	QString getNormalExpression();
	QSharedPointer<QRhiTextureDesc> getTexture(const QString& inName);
	QSharedPointer<QRhiUniformBlock> uniformBlock;
	QList<QSharedPointer<QRhiTextureDesc>> textureDescs;
};

class QENGINECORE_API QRhiMaterialGroup : public QObject {
	Q_OBJECT
public:
	QRhiMaterialGroup(QSharedPointer<QMaterial> inMaterial);
	QRhiMaterialGroup(QList<QSharedPointer<QMaterial>> inMaterials);
	QSharedPointer<QRhiMaterialDesc> getMaterialDesc(int inIndex);
	const QList<QSharedPointer<QRhiMaterialDesc>>& getDescList() { return mDescList; }
protected:
	void addMaterial(QSharedPointer<QMaterial> inMaterial);
private:
	QMap<QSharedPointer<QMaterial>, QSharedPointer<QRhiMaterialDesc>> mInfoMap;
	QList<QSharedPointer<QRhiMaterialDesc>> mDescList;
};

Q_DECLARE_METATYPE(QRhiMaterialGroup*);
Q_DECLARE_METATYPE(QSharedPointer<QRhiMaterialGroup>);

#endif // QRhiMaterialGroup_h__
