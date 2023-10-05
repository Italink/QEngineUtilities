#ifndef QPrimitiveRenderProxy_h__
#define QPrimitiveRenderProxy_h__

#include <QObject>
#include "Render/RHI/QRhiUniformBlock.h"
#include "Render/RHI/QRhiMaterialGroup.h"

class IRenderer;

class QENGINECORE_API QRhiVertexInputAttributeEx : public QRhiVertexInputAttribute {
public:
	QRhiVertexInputAttributeEx(QString name, int binding, int location, Format format, quint32 offset, int matrixSlice = -1)
		: QRhiVertexInputAttribute(binding, location, format, offset, matrixSlice)
		, mName(name) {
	}
	QString mName;
};

class QENGINECORE_API QRhiVertexInputBindingEx :public QRhiVertexInputBinding {
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

class QENGINECORE_API QPrimitiveRenderProxy: public QObject{
	Q_OBJECT
	Q_PROPERTY(QMap<QString, QRhiUniformBlock*> UniformBlocks READ getUniformBlocks WRITE setUniformBlocks)
	Q_PROPERTY(QMap<QString, QRhiTextureDesc*> Textures READ getTextures WRITE setTextures)
public:
	QRhiSignal mSigRebuild;
	QRhiSignal mSigUpload;

	struct StageInfo {
		QList<QSharedPointer<QRhiUniformBlock>> uniformBlocks;
		QList<QSharedPointer<QRhiTextureDesc>> textureDescs;
		QByteArray versionCode = "#version 440\n";
		QByteArray defineCode;
		QByteArray mainCode;
	};

	struct UpdateContext {
		QMatrix4x4 viewMatrix;
		QMatrix4x4 projectionMatrix;
		QMatrix4x4 projectionMatrixWithCorr;
	};

	struct Employee {
		QSharedPointer<QRhiGraphicsPipeline> pipeline;
		QRhiTextureRenderTarget* renderTarget = nullptr;
		std::function<void(QRhiGraphicsPipeline*)> postSetup;
	};

	struct UniformBlocks{
		UniformBlocks(QPrimitiveRenderProxy* builder) :mBuilder(builder) {}
		QRhiUniformBlock* operator [](const QString& inName) const { return mBuilder->getUniformBlock(inName); }
	private:
		QPrimitiveRenderProxy* mBuilder = nullptr;
	};

	QPrimitiveRenderProxy(IRenderComponent* inRenderComponent);
	~QPrimitiveRenderProxy();

	QRhiGraphicsPipeline::Topology getTopology() const { return mTopology; }
	void setTopology(QRhiGraphicsPipeline::Topology val) {mTopology = val;  mSigRebuild.request();}

	QRhiGraphicsPipeline::CullMode getCullMode() const { return mCullMode; }
	void setCullMode(QRhiGraphicsPipeline::CullMode val) { mCullMode = val; mSigRebuild.request(); }

	QRhiGraphicsPipeline::FrontFace getFrontFace() const { return mFrontFace; }
	void setFrontFace(QRhiGraphicsPipeline::FrontFace val) { mFrontFace = val; mSigRebuild.request(); }

	QVector<QRhiGraphicsPipeline::TargetBlend> getBlendStates() const { return mBlendStates; }
	void setBlendStates(QVector<QRhiGraphicsPipeline::TargetBlend> val) { mBlendStates = val; }

	bool isDepthTestEnabled() const { return bEnableDepthTest; }
	void setDepthTest(bool val) { bEnableDepthTest = val; mSigRebuild.request(); }

	bool isDepthWriteEnabled() const { return bEnableDepthWrite; }
	void setDepthWrite(bool val) { bEnableDepthWrite = val; mSigRebuild.request();  }

	QRhiGraphicsPipeline::CompareOp getDepthTestOp() const { return mDepthTestOp; }
	void setDepthTestOp(QRhiGraphicsPipeline::CompareOp val) { mDepthTestOp = val; mSigRebuild.request();  }

	bool isStencilTestEnabled() const { return bEnableStencilTest; }
	void setStencilTest(bool val) { bEnableStencilTest = val;  mSigRebuild.request();  }

	QRhiGraphicsPipeline::StencilOpState getStencilFrontOp() const { return mStencilFrontOp; }
	void setStencilFrontOp(QRhiGraphicsPipeline::StencilOpState val) { mStencilFrontOp = val; mSigRebuild.request();  }

	QRhiGraphicsPipeline::StencilOpState getStencilBackOp() const { return mStencilBackOp; }
	void setStencilBackOp(QRhiGraphicsPipeline::StencilOpState val) { mStencilBackOp = val; mSigRebuild.request();  }

	quint32 getStencilReadMask() const { return mStencilReadMask; }
	void setStencilReadMask(quint32 val) { mStencilReadMask = val;  mSigRebuild.request(); }

	quint32 getStencilWriteMask() const { return mStencilWriteMask; }
	void setStencilWriteMask(quint32 val) { mStencilWriteMask = val; mSigRebuild.request();  }

	float getLineWidth() const { return mLineWidth; }
	void setLineWidth(float val) { mLineWidth = val; mSigRebuild.request();  }

	QRhiGraphicsPipeline::PolygonMode getPolygonMode() const { return mPolygonMode; }
	void setPolygonMode(QRhiGraphicsPipeline::PolygonMode val) { mPolygonMode = val; mSigRebuild.request();  }

	QMap<QString, QRhiUniformBlock*> getUniformBlocks() { return mUniformMap; }
	void setUniformBlocks(QMap<QString, QRhiUniformBlock*>) {}

	QMap<QString, QRhiTextureDesc*> getTextures() { return mTextureMap; }
	void setTextures(QMap<QString, QRhiTextureDesc*>) {}
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
		QRhiSampler::Filter mipmapMode = QRhiSampler::Filter::None,
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
		QRhiSampler::Filter mipmapMode = QRhiSampler::Filter::None,
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
		QRhiSampler::Filter mipmapMode = QRhiSampler::Filter::None,
		QRhiSampler::AddressMode addressU = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::AddressMode::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::AddressMode::Repeat);

	void addTextureDesc(QRhiShaderStage::Type inStage, QSharedPointer<QRhiTextureDesc> inTexture);

	void setTexture(const QString& inName, const QImage& inImage);

	QRhiShaderResourceBindings* getShaderResourceBindings() const;
	QRhiGraphicsPipeline* getGraphicsPipeline() const;
	IRenderComponent* getRenderComponent() const;

	void setOnUpload(std::function<void(QRhiResourceUpdateBatch* batch)> callback) { mUploadCallback = callback; }
	void setOnUpdate(std::function<void(QRhiResourceUpdateBatch* batch, const UniformBlocks&, const UpdateContext&)> callback) { mUpdateCallback = callback; }
	void setOnDraw(std::function<void(QRhiCommandBuffer* cmdBuffer)> callback) { mDrawCallback = callback; }

	void tryCreate(QRhiTextureRenderTarget* renderTarget);
	void tryUpload(QRhiResourceUpdateBatch* batch);
	void update(QRhiResourceUpdateBatch* batch,const UpdateContext& ctx);
	void draw(QRhiCommandBuffer* cmdBuffer);

	bool hasEmployee(const QString& inName); 
	QRhiGraphicsPipeline* gerEmployee(const QString& inName);
	QRhiGraphicsPipeline* createEmployee(const QString& inName, QRhiTextureRenderTarget* renderTarget, std::function<void(QRhiGraphicsPipeline*)> postSetup);
private:
	void recreateEmployee(Employee& inEmployee);

	QByteArray getInputFormatTypeName(QRhiVertexInputAttribute::Format inFormat);
	QByteArray getOutputFormatTypeName(QRhiTexture::Format inFormat);
	void recreateShaderBindings(QRhiTextureRenderTarget* inRenderTarget, QRhi *inRhi);
private:
	IRenderComponent* mRenderComponent = nullptr;
	bool bIsUploaded = false;
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

	std::function<void(QRhiResourceUpdateBatch* batch)> mUploadCallback;
	std::function<void(QRhiResourceUpdateBatch* batch, const UniformBlocks&, const UpdateContext&)> mUpdateCallback;
	std::function<void(QRhiCommandBuffer* cmdBuffer)> mDrawCallback;

	QMap<QString, Employee> mEmployeeMap;
};

Q_DECLARE_METATYPE(QPrimitiveRenderProxy*);
Q_DECLARE_METATYPE(QSharedPointer<QPrimitiveRenderProxy>);

#endif // QPrimitiveRenderProxy_h__
