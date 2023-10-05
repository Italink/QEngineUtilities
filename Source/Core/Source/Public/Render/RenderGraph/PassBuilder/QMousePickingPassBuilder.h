#ifndef QMousePickingPassBuilder_h__
#define QMousePickingPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
class IRenderComponent;

class QENGINECORE_API QMousePickingPassBuilder: public QObject , public IRenderPassBuilder{
	Q_OBJECT
	QRP_INPUT_BEGIN(QMousePickingPassBuilder)
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QMousePickingPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef,SelectMask)
	QRP_OUTPUT_END()
public:
	QMousePickingPassBuilder();
	void requestPick(QPoint point);
Q_SIGNALS:
	void componentSelected(IRenderComponent*);
private:
	QRhiSignal mSigPick;
	QPoint mReadPoint;	
	QRhiTextureRef mDSTexture;
	QRhiBufferRef mReadbackBuffer;
	QRhiTextureRenderTargetRef mRenderTarget;
	QRhiReadbackResult mReadbackResult;

	QShader mSelectMaskFS;
	QRhiShaderResourceBindingsRef mSelectMaskBindings;
	QRhiGraphicsPipelineRef mSelectMaskPipeline;
protected:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;

	void makeCopyTextureToBufferCommand(QRhiCommandBuffer* cmdBuffer);
};

#endif // QMousePickingPassBuilder_h__
