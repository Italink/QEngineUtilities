#ifndef IRenderPass_h__
#define IRenderPass_h__

#include "Render/RHI/QRhiEx.h"
#include "IRenderer.h"
#include "IRenderComponent.h"
#include "Utils/QObjectBuilder.h"

class QSkyboxRenderComponent;

#define Q_EXPAND_INPUT_TEXTURE_GETTER(Name) QRhiTexture* getTextureIn_##Name(){ return getInputTexture(#Name); }
#define Q_EXPAND_INPUT_TEXTURE_SETTER(Name) __Builder& setTextureIn_##Name(const QString& inPassName, int inSlot){ mObject->registerInputTextureLink(#Name,inPassName,inSlot); return *this; }
#define Q_EXPAND_OUTPUR_TEXTURE_REGISTER(Name) void registerTextureOut_##Name(QRhiTexture* inTexture){ registerOutputTexture(Out::Name,#Name,inTexture); }
#define Q_EXPAND_OUTPUR_TEXTURE_SLOT(Name) Name,

#define Q_BUILDER_BEGIN_RENDER_PASS(ClassType,...) \
	protected: \
		Q_MACRO_EXPAND(Q_MACRO_PASTE(Q_EXPAND_INPUT_TEXTURE_GETTER, __VA_ARGS__)) \
	public: \
		Q_BUILDER_BEGIN(ClassType) \
		Q_MACRO_EXPAND(Q_MACRO_PASTE(Q_EXPAND_INPUT_TEXTURE_SETTER, __VA_ARGS__)) 

#define Q_BUILDER_BEGIN_RENDER_PASS_WITHOUT_IN(ClassType) \
		Q_BUILDER_BEGIN(ClassType) 

#define Q_BUILDER_END_RENDER_PASS(...) \
		Q_BUILDER_END() \
		enum Out{\
			Q_MACRO_EXPAND(Q_MACRO_PASTE(Q_EXPAND_OUTPUR_TEXTURE_SLOT, __VA_ARGS__)) \
		}; \
	protected: \
		Q_MACRO_EXPAND(Q_MACRO_PASTE(Q_EXPAND_OUTPUR_TEXTURE_REGISTER, __VA_ARGS__))

class QENGINECORE_API IRenderPass: public QObject{
	friend class QFrameGraph;
public:
	virtual ~IRenderPass(){}

	virtual void setRenderer(IRenderer* inRenderer);
	IRenderer* getRenderer() const { return mRenderer; }

	const QSet<QString>& getDependentPassNodeNames();

	void requestCompile(bool bForce = false);

	int getOutputTextureSize();
	QRhiTexture* getOutputTexture(const int& inSlot);
	QRhiTexture* getOutputTexture(const QString& inName);
	QList<QRhiTexture*> getOutputTextures();
	QRhiTexture* getFirstOutputTexture();
	const QMap<QString, QPair<QString, int>>& getInputTextureLinks();
protected:
	virtual void compile() {};
	virtual void resizeAndLinkNode(const QSize& size) {}
	virtual void render(QRhiCommandBuffer* cmdBuffer) = 0;

	QRhiTexture* getInputTexture(const QString& inName);
	void registerOutputTexture(int inSlot, const QString& inName, QRhiTexture* inTexture);
	void registerInputTextureLink(const QString& inTexName, const QString& inPassName, int inSlot);
protected:
	IRenderer* mRenderer = nullptr;
	QRhiEx* mRhi;
	QMap<int, QRhiTexture*> mOutputTexutres;
	QMap<QString, QPair<QString, int>> mInputTextureLinks;
private:
	QSet<QString> mDependentPassNodeNames;
	bool bCompiled = false;
};

#define Q_BUILDER_BEGIN_BASE_PASS(ClassType) \
	public: \
		Q_BUILDER_BEGIN(ClassType) \
		__Builder& addComponent(IRenderComponent* inRenderComponent) { mObject->addRenderComponent(inRenderComponent); return *this; } \

#define Q_BUILDER_END_BASE_PASS(...) \
		Q_BUILDER_END() \
		enum Out{ \
			Q_MACRO_EXPAND(Q_MACRO_PASTE(Q_EXPAND_OUTPUR_TEXTURE_SLOT, __VA_ARGS__)) \
		}; \
	protected: \
		Q_MACRO_EXPAND(Q_MACRO_PASTE(Q_EXPAND_OUTPUR_TEXTURE_REGISTER, __VA_ARGS__))


class QENGINECORE_API IBasePass :public IRenderPass {
	Q_OBJECT
public:
	void setSampleCount(int inSampleCount) { mSampleCount = inSampleCount;}
	int getSampleCount() { return mSampleCount; }
	virtual QRhiRenderPassDescriptor* getRenderPassDescriptor() = 0;
	virtual QRhiRenderTarget* getRenderTarget() = 0;

	void setRenderer(IRenderer* inRenderer) override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	QList<QPair<QRhiTexture::Format, QString>> getRenderTargetColorAttachments();
	bool hasColorAttachment(const QString& inName);

	void addRenderComponent(IRenderComponent* inRenderComponent,int inIndex = -1);
protected:
	QVector<IRenderComponent*> mRenderComponents;
	int mSampleCount = 1;
};

#endif // IRenderPass_h__
