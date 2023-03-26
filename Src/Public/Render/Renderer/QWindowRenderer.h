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
	QWindow* getWindow() override;
	QRhiWindow* getRhiWindow() const;
	QRhiRenderTarget* renderTaget() override;
	int sampleCount() override;
protected:
	void render() override;
	void resize(const QSize& size) override;
	QRhiCommandBuffer* commandBuffer() override;
private:
	QRhiWindow* mWindow;
#ifdef QENGINE_WITH_EDITOR
	QScopedPointer<QDebugUIPainter> mDebugUiPainter;
	QRhiTexture* mOverrideOutputTexture = nullptr;
#endif // QENGINE_WITH_EDITOR
};


#endif // QWindowRenderer_h__
