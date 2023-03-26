#ifndef QImGUIRenderPass_h__
#define QImGUIRenderPass_h__

#include "Render/IRenderPass.h"
#include "Render/Painter/ImGuiPainter.h"

class QImGUIRenderPass : public IRenderPass {
	Q_OBJECT

	Q_BUILDER_BEGIN(QImGUIRenderPass)
		Q_BUILDER_FUNCTION_BEGIN(setPaintFunctor, std::function<void()> val)
			Q_BUILDER_OBJECT_PTR->setPaintFunctor(val);
		Q_BUILDER_FUNCTION_END()
	Q_BUILDER_END_RENDER_PASS(UiLayer)
public:
	QImGUIRenderPass();
	void setPaintFunctor(std::function<void()> val);
protected:
	void resizeAndLinkNode(const QSize& size) override;
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
