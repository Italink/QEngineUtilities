#ifndef IMeshPassBuilder_h__
#define IMeshPassBuilder_h__

#include "Render/RenderGraph/IRGPassBuilder.h"
#include "Render/IRenderComponent.h"

class QENGINECORE_API IMeshPassBuilder : public IRGPassBuilder {
	friend class QRGBuilder;
public:
	struct InputParams{
		QVector<IRenderComponent*> components;
	}Input;

	struct OutputParams{
		QRhiTextureRef baseColor;
	}Output;
protected:
	void setup(QRGBuilder& builder);
	void execute(QRhiCommandBuffer* cmdBuffer);
private:
	QRhiTextureRenderTargetRef mRenderTarget;
	QRhiRenderBufferRef mDepthStencilBuffer;
};

#endif // IMeshPassBuilder_h__
