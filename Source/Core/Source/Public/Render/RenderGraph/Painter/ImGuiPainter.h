#ifndef ImGuiPainter_h__
#define ImGuiPainter_h__

#include <QObject>
#include <QMutex>
#include "imgui_internal.h"
#include "IPainter.h"
#include "QEngineCoreAPI.h"

class QENGINECORE_API ImGuiPainter :public QObject ,public IPainter{
	Q_OBJECT
public:
	ImGuiPainter();
	void setupWindow(QWindow* window);
	void setupPaintFunctor(std::function<void(ImGuiContext*)> val) { mPaintFunctor = val; }
	void registerImage(const QByteArray& inName, const QImage& inImage);
	ImTextureID getImageId(const QByteArray& inName);

	void setup(QRenderGraphBuilder& builder, QRhiRenderTarget* rt) override;
	void resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi) override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;

protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	QWindow* getWindow() const { return mWindow; }
	void tryRebuildFontTexture();
protected:
	struct LocalImage {
		QImage mImage;
		QSize mSize;
		QRhiTextureRef mTexture;
		QRhiShaderResourceBindingsRef mBindings;
	};
	QMap<QByteArray, LocalImage> mRegisterImages;

	QShader mImGuiVS;
	QShader mImGuiFS;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiShaderResourceBindingsRef mDynamicBindings;
	QRhiBufferRef mVertexBuffer;
	QRhiBufferRef mIndexBuffer;
	QRhiBufferRef mUniformBuffer;
	QRhiSamplerRef mSampler;
	QWindow* mWindow;
	ImGuiContext* mImGuiContext = nullptr;
	double       mTime = 0.0f;
	std::function<void(ImGuiContext*)> mPaintFunctor;
	QMutex mMutex;
};

#endif // ImGuiPainter_h__