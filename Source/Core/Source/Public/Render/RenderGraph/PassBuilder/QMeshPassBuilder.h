#ifndef QMeshPassBuilder_h__
#define QMeshPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
#include "Render/IRenderComponent.h"

class QENGINECORE_API QMeshPassBuilder : public IRenderPassBuilder {
	friend class QRenderGraphBuilder;
public:
	QRP_INPUT_BEGIN(QMeshPassBuilder)
		QRP_INPUT_ATTR(QVector<IRenderComponent*>, Components);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QMeshPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, BaseColor)
	QRP_OUTPUT_END()
protected:
	void setup(QRenderGraphBuilder& builder);
	void execute(QRhiCommandBuffer* cmdBuffer);
private:
	QRhiTextureRenderTargetRef mRenderTarget;
	QRhiRenderBufferRef mDepthStencilBuffer;
};

#endif // QMeshPassBuilder_h__
