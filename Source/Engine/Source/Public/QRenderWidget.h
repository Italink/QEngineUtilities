#ifndef QRenderWidget_h__
#define QRenderWidget_h__

#include "QWidget"
#include "QEngineUtilitiesAPI.h"

class QFrameGraph;
class QInnerRhiWindow;
class QWindowRenderer;
class QDetailView;
class IRenderer;

class QENGINEUTILITIES_API QRenderWidget :public QWidget {
public:
	QRenderWidget(IRenderer* renderer);
protected:
#ifdef QENGINE_WITH_EDITOR
	void keyPressEvent(QKeyEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;
#endif 
protected:
	QWidget* mViweport = nullptr;
	IRenderer* mRenderer;
#ifdef QENGINE_WITH_EDITOR
	QDetailView* mDetailView;
#endif 
};

#endif // QRenderWidget_h__
