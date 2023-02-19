#ifndef QWindowRenderer_h__
#define QWindowRenderer_h__

#include "Render/IRenderer.h"

class QRhiWindow;
class QDebugUIPainter;

class QWindowRenderer : public IRenderer {
	Q_OBJECT
public:
	friend class QRenderWidget;
	QWindowRenderer(QRhiWindow* inWindow);
	QWindow* getWindow() const;
	QRhiRenderTarget* renderTaget() override;
	int sampleCount() override;
protected:
	void render() override;
	QRhiCommandBuffer* commandBuffer() override;
	void refreshOutputTexture() override;
private:
	QRhiWindow* mWindow;
#ifdef QENGINE_WITH_EDITOR
	QScopedPointer<QDebugUIPainter> mDebugUiPainter;
#endif // QENGINE_WITH_EDITOR
};


#endif // QWindowRenderer_h__
