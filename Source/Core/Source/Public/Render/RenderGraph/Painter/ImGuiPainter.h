#ifndef ImGuiPainter_h__
#define ImGuiPainter_h__

#include "QObject"
#include "imgui_internal.h"
#include "IPainter.h"
#include "QEngineCoreAPI.h"

class QENGINECORE_API ImGuiPainter :public QObject ,public IPainter{
	Q_OBJECT
public:
	ImGuiPainter();
	void setupWindow(QWindow* window);
	void setupPaintFunctor(std::function<void(ImGuiContext*)> val) { mPaintFunctor = val; }
	void registerImage(const QString& inName, const QImage& inImage);
	ImTextureID getImageId(const QString& inName);

	void setup(QRenderGraphBuilder& builder, QRhiRenderTargetRef rt) override;
	void resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi) override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
protected:
	struct LocalImage {
		QImage mImage;
		QRhiTextureRef mTexture;
	};
	QMap<QString, LocalImage> mRegisterImages;

	QShader mImGuiVS;
	QShader mImGuiFS;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiBufferRef mVertexBuffer;
	QRhiBufferRef mIndexBuffer;
	QRhiBufferRef mUniformBuffer;
	QRhiShaderResourceBindingsRef mBindings;
	QRhiTextureRef mFontTexture;
	QRhiSamplerRef mSampler;
	QWindow* mWindow;
	QImage mFontImage;
	ImGuiContext* mImGuiContext = nullptr;
	double       mTime = 0.0f;
	bool         mMousePressed[3] = { false, false, false };
	float        mMouseWheel = 0;
	float        mMouseWheelH = 0;
	std::function<void(ImGuiContext*)> mPaintFunctor;
};

#endif // ImGuiPainter_h__