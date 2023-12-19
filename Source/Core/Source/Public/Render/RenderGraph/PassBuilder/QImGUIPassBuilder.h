#ifndef QImGUIPassBuilder_h__
#define QImGUIPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
#include "Render/RenderGraph/Painter/ImGuiPainter.h"

class QENGINECORE_API QImGUIPassBuilder : public IRenderPassBuilder {
public:
	QImGUIPassBuilder();

	QRP_INPUT_BEGIN(QImGUIPassBuilder)
		QRP_INPUT_ATTR(std::function<void(ImGuiContext*)>, PaintFunctor);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QImGUIPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, ImGuiTexture);
	QRP_OUTPUT_END()
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;

private:
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	RTResource mRT;
	QScopedPointer<ImGuiPainter> mPainter;
};

#endif // QImGUIPassBuilder_h__
