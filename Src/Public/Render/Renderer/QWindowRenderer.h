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
	QRhiWindow* getWindow() const;
	QRhiRenderTarget* renderTaget() override;
	int sampleCount() override;
#ifdef QENGINE_WITH_EDITOR
	bool TryOverrideOutputTexture(QRhiTexture* inTexture);
#endif 
protected:
	void render() override;
	QRhiCommandBuffer* commandBuffer() override;
	void refreshOutputTexture() override;
private:
	QRhiWindow* mWindow;
#ifdef QENGINE_WITH_EDITOR
	QScopedPointer<QDebugUIPainter> mDebugUiPainter;
	QRhiTexture* mOverrideOutputTexture = nullptr;
#endif // QENGINE_WITH_EDITOR
};


#endif // QWindowRenderer_h__
