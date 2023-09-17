#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>
#include "QRhiHelper.h"

class QENGINECORE_API QRhiWindow :public QWindow {
	friend class QWindowRenderer;
public:
	struct InitParams {
		QRhi::Implementation backend = QRhi::Vulkan;
		QRhi::Flags rhiFlags = QRhi::Flag();
		QRhiSwapChain::Flags swapChainFlags = QRhiSwapChain::Flag::NoVSync;
		QRhi::BeginFrameFlags beginFrameFlags;
		QRhi::EndFrameFlags endFrameFlags;
		int sampleCount = 1;
		bool enableStat = false;
	};
	QRhiWindow(QRhiWindow::InitParams inInitParmas);
	~QRhiWindow();
	QRhi* getRhi() const { return mRhi.get(); }
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
	virtual void onResize(const QSize& inSize) {}
	virtual void onExit() {}
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent*) override;
private:
	InitParams mInitParams;

	bool mRunning = false;
	bool mNotExposed = false;
	bool mNewlyExposed = false;
	bool mHasSwapChain = false;
	bool mNeedResize = false;

	int mFps = 0;
	float mCpuFrameTime;
	QElapsedTimer mCpuFrameTimer;
	float mGpuFrameTime;
protected:
	QSharedPointer<QRhi> mRhi;
	QScopedPointer<QRhiSwapChain> mSwapChain;
	QScopedPointer<QRhiRenderBuffer> mDSBuffer  ;
	QScopedPointer<QRhiRenderPassDescriptor> mSwapChainPassDesc;
};

#endif // QRhiWindow_h__
