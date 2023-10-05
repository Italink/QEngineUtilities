#ifndef QRendererSurface_h__
#define QRendererSurface_h__

#include <QWindow>
#include "RenderGraph/QRenderGraphBuilder.h"
#include "IRenderer.h"

class IRendererSurface {
public:
	virtual QWindow* maybeWindow() { return nullptr; }
	virtual void resize(const QSize& size) = 0;

	virtual void initialize(QRhi* rhi, QRhiHelper::InitParams initParams) = 0;
	virtual void destroy() = 0;

	virtual bool beginFrame(QRhiCommandBuffer** outCmdBuffer, QRhiRenderTarget** outRenderTarget) = 0;
	virtual void endFrame() = 0;
};

class QRendererWindowSurface: public QWindow, public IRendererSurface
{
	Q_OBJECT
public:
	QRendererWindowSurface(QRhi::Implementation impl, QSize size);
protected:
	bool mRunning = false;
	bool mNotExposed = true;
	bool mNewlyExposed = false;
	bool mHasSwapChain = false;
	QRhi* mRhi = nullptr;
	QRhi::BeginFrameFlags mBeginFrameFlags;
	QRhi::EndFrameFlags mEndFrameFlags;
	QScopedPointer<QRhiSwapChain> mSwapChain;
	QScopedPointer<QRhiRenderBuffer> mDSBuffer;
	QScopedPointer<QRhiRenderPassDescriptor> mSwapChainPassDesc;
Q_SIGNALS:
	void initRequested();
	void renderRequested();
	void surfaceGoingAway();
	void syncSurfaceSizeRequested();
private:
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent* e) override;

	QWindow* maybeWindow() override;
	void resize(const QSize& size) override;

	void initialize(QRhi* rhi, QRhiHelper::InitParams initParams) override;
	void destroy() override;
	bool beginFrame(QRhiCommandBuffer** outCmdBuffer, QRhiRenderTarget** outRenderTarget) override;
	void endFrame() override;
};

class QRendererOffscreenSurface: public IRendererSurface {
public:
	QRendererOffscreenSurface(QSize size);
private:
	QRhi* mRhi = nullptr;
	QSize mRequestSize;
	int mRequestSampleCount = 1;
	QRhi::BeginFrameFlags mBeginFrameFlags;
	QRhi::EndFrameFlags mEndFrameFlags;
	QScopedPointer<QRhiTexture> mColorAttachment;
	QScopedPointer<QRhiTexture> mDepthStencilAttachment;
	QScopedPointer<QRhiTextureRenderTarget> mRenderTarget;
	QScopedPointer<QRhiRenderPassDescriptor> mRenderPassDesc;
protected:
	void resize(const QSize& size) override;
	void initialize(QRhi* rhi, QRhiHelper::InitParams initParams) override;
	void destroy() override;
	bool beginFrame(QRhiCommandBuffer** outCmdBuffer, QRhiRenderTarget** outRenderTarget) override;
	void endFrame() override;
};


#endif // QRendererSurface_h__
