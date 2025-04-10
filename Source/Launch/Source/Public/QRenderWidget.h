#ifndef QRenderWidget_h__
#define QRenderWidget_h__

#include "QWidget"
#include "QEngineLaunchAPI.h"

class QFrameGraph;
class QInnerRhiWindow;
class QWindowRenderer;
class QDetailView;
class IRenderer;

class QENGINELAUNCH_API QRenderWidget :public QWidget {
public:
	QRenderWidget(IRenderer* renderer);
protected:
	void keyPressEvent(QKeyEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;
protected:
	QWidget* mViewport = nullptr;
	IRenderer* mRenderer;
#ifdef QENGINE_WITH_EDITOR
	QDetailView* mDetailView;
#endif 
};

#endif // QRenderWidget_h__
