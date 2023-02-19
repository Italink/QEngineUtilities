#ifndef QRenderWidget_h__
#define QRenderWidget_h__

#include "QWidget"
#include "Utils/QCamera.h"
#include "Render/RHI/QRhiWindow.h"

class QFrameGraph;
class QInnerRhiWindow;
class QWindowRenderer;
class QDetailView;
class QObjectTreeView;

class QRenderWidget :public QWidget {
public:
	friend class QInnerRhiWindow;
	QRhiEx::Signal sigRecompileRenderer;
public:
	QRenderWidget(QRhiWindow::InitParams inInitParams);
	QCamera* setupCamera();
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);
	void requestCompileRenderer();
	QWindow* getRhiWindow();
protected:
	void onInit();
	void onRenderTick();
	void onResizeEvent(const QSize& inSize);
	void onExit();
	void keyPressEvent(QKeyEvent* event) override;
protected:
	QRhiWindow::InitParams mInitParams;
	QInnerRhiWindow* mRhiWindow = nullptr;
	QCamera* mCamera;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QSharedPointer<QWindowRenderer> mRenderer;
#ifdef QENGINE_WITH_EDITOR
	QObjectTreeView* mObjectTreeView;
	QDetailView* mDetailView;
#endif // QENGINE_WITH_EDITOR
};

#endif // QRenderWidget_h__
