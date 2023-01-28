#ifndef QRenderViewport_h__
#define QRenderViewport_h__

#include "QWidget"
#include "RHI/QRhiWindow.h"

class QCamera;
class QFrameGraph;
class QInnerRhiWindow;
class QWindowRenderer;
class QDetailView;
class QObjectTreeView;

class QRenderViewport :public QWidget {
public:
	QRhiEx::Signal sigRecompileRenderer;
public:
	QRenderViewport(QRhiWindow::InitParams inInitParams);
	QCamera* setupCamera();
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);
	void requestCompileRenderer();
protected:
	QRhiWindow::InitParams mInitParams;
	QInnerRhiWindow* mRhiWindow = nullptr;
	QWindowRenderer* mRenderer = nullptr;
	QCamera* mCamera;
	QSharedPointer<QFrameGraph> mFrameGraph;
#ifdef WITH_EDITOR
	QObjectTreeView* mObjectTreeView;
	QDetailView* mDetailView;
#endif // WITH_EDITOR
};

#endif // QRenderViewport_h__
