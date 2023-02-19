#ifndef QImGUIRenderPass_h__
#define QImGUIRenderPass_h__

#include "Render/IRenderPass.h"
#include "Render/Painter/ImGuiPainter.h"

class QImGUIRenderPass : public IRenderPassBase {
	Q_OBJECT
public:
	enum OutSlot {
		UiLayer
	};
	QImGUIRenderPass();
	QImGUIRenderPass* setupPaintFunctor(std::function<void()> val);
protected:
	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<ImGuiPainter> mPainter;
};

#endif // QImGUIRenderPass_h__
