#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>
#include "QRhiEx.h"

class QRhiWindow :public QWindow {
	friend class QWindowRenderer;
public:
	struct InitParams {
		QRhi::Implementation backend = QRhi::Vulkan;
		QRhi::Flags rhiFlags;
		QRhiSwapChain::Flags swapChainFlags = QRhiSwapChain::Flag::NoVSync;
		QRhi::BeginFrameFlags beginFrameFlags;
		QRhi::EndFrameFlags endFrameFlags;
		int sampleCount = 1;
		bool enableStat = false;
	};
	QRhiWindow(QRhiWindow::InitParams inInitParmas);
	~QRhiWindow();
	QRhiEx* getRhi() const { return mRhi.get(); }
	int getFps() { return mFps; }
	float getCpuFrameTime() const { return mCpuFrameTime; }
	float getGpuFrameTime() const { return mGpuFrameTime; }
private:
	void initializeInternal();
	void renderInternal();
	void resizeInternal();
protected:
	virtual void onInit(){}
	virtual void onRenderTick() {}
	virtual void onResizeEvent(const QSize& inSize) {}
	virtual void onExit() {}
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent*) override;
private:
	InitParams mInitParams;

	bool mRunning = false;
	bool mNotExposed = false;
	bool mNewlyExposed = false;
	bool mHasSwapChain = false;

	int mFps = 0;
	float mCpuFrameTime;
	QElapsedTimer mCpuFrameTimer;
	float mGpuFrameTime;
protected:
	QScopedPointer<QRhiEx> mRhi;
	QScopedPointer<QRhiSwapChain> mSwapChain;
	QScopedPointer<QRhiRenderBuffer> mDSBuffer  ;
	QScopedPointer<QRhiRenderPassDescriptor> mSwapChainPassDesc;
};

#endif // QRhiWindow_h__
