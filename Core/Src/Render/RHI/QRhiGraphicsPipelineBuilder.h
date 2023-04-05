#ifndef QRhiGraphicsPipelineBuilder_h__
#define QRhiGraphicsPipelineBuilder_h__

#include "Render/RHI/QRhiUniformBlock.h"
#include "QObject"
#include "Utils/QMetaData.h"
#include "QRhiMaterialGroup.h"

class QRhiVertexInputAttributeEx : public QRhiVertexInputAttribute {
public:
	QRhiVertexInputAttributeEx(QString name, int binding, int location, Format format, quint32 offset, int matrixSlice = -1)
		: QRhiVertexInputAttribute(binding, location, format, offset, matrixSlice)
		, mName(name) {
	}
	QString mName;
};

class QRhiVertexInputBindingEx :public QRhiVertexInputBinding {
public:
	QRhiVertexInputBindingEx(QRhiBuffer* buffer, quint32 stride, int offset = 0, Classification cls = PerVertex, int stepRate = 1)
		: QRhiVertexInputBinding(stride, cls, stepRate)
		, mBuffer(buffer)
		, mOffset(offset)
	{
	}
	QRhiBuffer* mBuffer;
	int mOffset = 0;
};

class QRhiGraphicsPipelineBuilder: public QObject{
	Q_OBJECT
		Q_PROPERTY(QMap<QString, QRhiUniformBlock*> UniformBlocks READ getUniformBlocks WRITE setUniformBlocks)
		Q_PROPERTY(QMap<QString, QRhiTextureDesc*> Textures READ getTextures WRITE setTextures)
public:
	inline static QList<QRhiGraphicsPipelineBuilder*> Instances;
	QRhiEx::Signal sigRebuild;

	struct StageInfo {
		QList<QSharedPointer<QRhiUniformBlock>> uniformBlocks;
		QList<QSharedPointer<QRhiTextureDesc>> textureDescs;
		QByteArray versionCode = "#version 440\n";
		QByteArray defineCode;
		QByteArray mainCode;
	};

	QRhiGraphicsPipelineBuilder(QObject* parent = nullptr) {
		Instances << this;
		setParent(parent);
	}
	~QRhiGraphicsPipelineBuilder() {
		Instances.removeOne(this);
	}

	QRhiGraphicsPipeline::Topology getTopology() const { return mTopology; }
	void setTopology(QRhiGraphicsPipeline::Topology val) {mTopology = val;  sigRebuild.request();}

	QRhiGraphicsPipeline::CullMode getCullMode() const { return mCullMode; }
	void setCullMode(QRhiGraphicsPipeline::CullMode val) { mCullMode = val; sigRebuild.request(); }

	QRhiGraphicsPipeline::FrontFace getFrontFace() const { return mFrontFace; }
	void setFrontFace(QRhiGraphicsPipeline::FrontFace val) { mFrontFace = val; sigRebuild.request(); }

	bool isDepthTestEnabled() const { return bEnableDepthTest; }
	void setDepthTest(bool val) { bEnableDepthTest = val; sigRebuild.request(); }

	bool isDepthWriteEnabled() const { return bEnableDepthWrite; }
	void setDepthWrite(bool val) { bEnableDepthWrite = val; sigRebuild.request();  }

	QRhiGraphicsPipeline::CompareOp getDepthTestOp() const { return mDepthTestOp; }
	void setDepthTestOp(QRhiGraphicsPipeline::CompareOp val) { mDepthTestOp = val; sigRebuild.request();  }

	bool isStencilTestEnabled() const { return bEnableStencilTest; }
	void setStencilTest(bool val) { bEnableStencilTest = val;  sigRebuild.request();  }

	QRhiGraphicsPipeline::StencilOpState getStencilFrontOp() const { return mStencilFrontOp; }
	void setStencilFrontOp(QRhiGraphicsPipeline::StencilOpState val) { mStencilFrontOp = val; sigRebuild.request();  }

	QRhiGraphicsPipeline::StencilOpState getStencilBackOp() const { return mStencilBackOp; }
	void setStencilBackOp(QRhiGraphicsPipeline::StencilOpState val) { mStencilBackOp = val; sigRebuild.request();  }

	quint32 getStencilReadMask() const { return mStencilReadMask; }
	void setStencilReadMask(quint32 val) { mStencilReadMask = val;  sigRebuild.request(); }

	quint32 getStencilWriteMask() const { return mStencilWriteMask; }
	void setStencilWriteMask(quint32 val) { mStencilWriteMask = val; sigRebuild.request();  }

	float getLineWidth() const { return mLineWidth; }
	void setLineWidth(float val) { mLineWidth = val; sigRebuild.request();  }

	QRhiGraphicsPipeline::PolygonMode getPolygonMode() const { return mPolygonMode; }
	void setPolygonMode(QRhiGraphicsPipeline::PolygonMode val) { mPolygonMode = val; sigRebuild.request();  }

	QMap<QString, QRhiUniformBlock*> getUniformBlocks() { return mUniformMap; }
	void setUniformBlocks(QMap<QString, QRhiUniformBlock*>) {}

	QMap<QString, QRhiTextureDesc*> getTextures() { return mTextureMap; }
	void setTextures(QMap<QString, QRhiTextureDesc*>) {}

	static void setPolygonModeOverride(QRhiGraphicsPipeline::PolygonMode inMode);
	static void clearPolygonModeOverride();
public:
	void setShaderMainCode(QRhiShaderStage::Type inStage, QByteArray inCode);

	void setInputAttribute(QVector<QRhiVertexInputAttributeEx> inInputAttributes);
	void setInputBindings(QVector<QRhiVertexInputBindingEx> inInputBindings);
	QVector<QRhiCommandBuffer::VertexInput> getVertexInputs();

	QRhiUniformBlock* addUniformBlock(QRhiShaderStage::Type inStage, const QString& inName);
	void addUniformBlock(QRhiShaderStage::Type inStage, QSharedPointer<QRhiUniformBlock> inUniformBlock);
	QRhiUniformBlock* getUniformBlock(const QString& inName);

	void addMaterial(QSharedPointer<QRhiMaterialDesc> inDesc);

	void addTexture2D(QRhiShaderStage::Type inStage,
		const QString& inName,
		const QImage& inImage, 
		QRhiSampler::Filter magFilter = QRhiSampler::Filter::Linear,
		QRhiSampler::Filter minFilter = QRhiSampler::Filter::Nearest,
		QRhiSampler::Filter mipmapMode = QRhiSampler::Filter::Linear,
		QRhiSampler::AddressMode addressU = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::AddressMode::Repeat);

	void addTexture2D(QRhiShaderStage::Type inStage,
		const QString& inName,
		QRhiTexture::Format inFormat,
		const QSize& inSize,
		const QByteArray& inData,
		QRhiSampler::Filter magFilter = QRhiSampler::Filter::Linear,
		QRhiSampler::Filter minFilter = QRhiSampler::Filter::Nearest,
		QRhiSampler::Filter mipmapMode = QRhiSampler::Filter::Linear,
		QRhiSampler::AddressMode addressU = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::AddressMode::Repeat);

	void addCubeMap(QRhiShaderStage::Type inStage,
		const QString& inName,
		QRhiTexture::Format inFormat,
		const QSize& inSize,
		const QVector<QByteArray>& inData,
		QRhiSampler::Filter magFilter = QRhiSampler::Filter::Linear,
		QRhiSampler::Filter minFilter = QRhiSampler::Filter::Nearest,
		QRhiSampler::Filter mipmapMode = QRhiSampler::Filter::Linear,
		QRhiSampler::AddressMode addressU = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::AddressMode::Repeat);

	void addTextureDesc(QRhiShaderStage::Type inStage, QSharedPointer<QRhiTextureDesc> inTexture);

	void setTexture(const QString& inName, const QImage& inImage);

	QRhiShaderResourceBindings* getShaderResourceBindings();
	QRhiGraphicsPipeline* getGraphicsPipeline() { return mPipeline.get(); }

	void create(IRenderComponent* inRenderComponent);
	void update(QRhiResourceUpdateBatch* batch);

	QByteArray getInputFormatTypeName(QRhiVertexInputAttribute::Format inFormat);
	QByteArray getOutputFormatTypeName(QRhiTexture::Format inFormat);
protected:
	void recreateShaderBindings(IRenderComponent* inRenderComponent, QRhiEx *inRhi);
private:
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QRhiGraphicsPipeline::Topology mTopology = QRhiGraphicsPipeline::Triangles;
	QRhiGraphicsPipeline::PolygonMode mPolygonMode = QRhiGraphicsPipeline::Fill;
	QRhiGraphicsPipeline::CullMode mCullMode = QRhiGraphicsPipeline::None;
	QRhiGraphicsPipeline::FrontFace mFrontFace = QRhiGraphicsPipeline::CCW;
	QVector<QRhiGraphicsPipeline::TargetBlend> mBlendStates;
	bool bEnableDepthTest = true;
	bool bEnableDepthWrite = true;
	QRhiGraphicsPipeline::CompareOp mDepthTestOp = QRhiGraphicsPipeline::Less;
	bool bEnableStencilTest = false;
	QRhiGraphicsPipeline::StencilOpState mStencilFrontOp;
	QRhiGraphicsPipeline::StencilOpState mStencilBackOp;
	quint32 mStencilReadMask = 0xFF;
	quint32 mStencilWriteMask = 0xFF;
	float mLineWidth = 1.0f;
	int mDepthBias = 0;
	float mSlopeScaledDepthBias = 0.0f;
	int mPatchControlPointCount = 3;
	QRhiVertexInputLayout mVertexInputLayout;
	QVector<QRhiVertexInputAttributeEx> mInputAttributes;
	QVector<QRhiVertexInputBindingEx> mInputBindings;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QHash<QRhiShaderStage::Type, StageInfo> mStageInfos;
	QMap<QString, QRhiUniformBlock*> mUniformMap;
	QMap<QString, QRhiTextureDesc*> mTextureMap;
	QList<QSharedPointer<QRhiSampler>> mSamplerList;
	inline static int PolygonModeOverride = -1;
};

Q_DECLARE_METATYPE(QRhiGraphicsPipelineBuilder*);
Q_DECLARE_METATYPE(QSharedPointer<QRhiGraphicsPipelineBuilder>);

#endif // QRhiGraphicsPipelineBuilder_h__
