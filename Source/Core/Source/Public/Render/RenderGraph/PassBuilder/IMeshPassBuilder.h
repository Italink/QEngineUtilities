#ifndef IMeshPassBuilder_h__
#define IMeshPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"

class QENGINECORE_API IMeshPassBuilder : public IRenderPassBuilder {
protected:
	virtual QRhiTextureRenderTarget* renderTarget() = 0;
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
};

#endif // IMeshPassBuilder_h__
