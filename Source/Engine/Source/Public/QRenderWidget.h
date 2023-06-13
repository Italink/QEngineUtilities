#ifndef QRenderWidget_h__
#define QRenderWidget_h__

#include "QWidget"
#include "Utils/QCamera.h"
#include "Render/RHI/QRhiWindow.h"
#include "QEngineUtilitiesAPI.h"

class QFrameGraph;
class QInnerRhiWindow;
class QWindowRenderer;
class QDetailView;
class QObjectTreeView;

class QENGINEUTILITIES_API QRenderWidget :public QWidget {
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
	void onResize(const QSize& inSize);
	void onExit();
#ifdef QENGINE_WITH_EDITOR
	void keyPressEvent(QKeyEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;
#endif 
protected:
	QRhiWindow::InitParams mInitParams;
	QInnerRhiWindow* mRhiWindow = nullptr;
	QCamera* mCamera;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QSharedPointer<QWindowRenderer> mRenderer;
#ifdef QENGINE_WITH_EDITOR
	QObjectTreeView* mObjectTreeView;
	QDetailView* mDetailView;
#endif 
};

#endif // QRenderWidget_h__
