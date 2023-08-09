#ifndef ImGuiPainter_h__
#define ImGuiPainter_h__

#include "QObject"
#include "imgui_internal.h"
#include "Render/IPainter.h"
#include "QEngineCoreAPI.h"

class QENGINECORE_API ImGuiPainter :public QObject ,public IPainter{
	Q_OBJECT
public:
	ImGuiPainter();
	void setupWindow(QWindow* window);
	void setupPaintFunctor(std::function<void(ImGuiContext*)> val) { mPaintFunctor = val; }
	void registerImage(const QString& inName, const QImage& inImage);
	ImTextureID getImageId(const QString& inName);
	void compile() override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
	void resourceUpdate(QRhiResourceUpdateBatch* batch) override;
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
protected:
	struct LocalImage {
		QImage mImage;
		QSharedPointer<QRhiTexture> mTexture;
	};
	QMap<QString, LocalImage> mRegisterImages;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QScopedPointer<QRhiTexture> mFontTexture;
	QScopedPointer<QRhiSampler> mSampler;
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