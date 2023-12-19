#ifndef QOutputPassBuilder_h__
#define QOutputPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
#include "QEngineLaunchAPI.h"
#include "Render/IRenderer.h"
#include "Render/Painter/DebugUiPainter.h"
#include "Render/RenderGraph/PassBuilder/QMousePickingPassBuilder.h"

class QENGINELAUNCH_API QOutputPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QOutputPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, InitialTexture);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QOutputPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, ToneMappingReslut);
	QRP_OUTPUT_END()
public:
	QOutputPassBuilder();  
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	QRhiRenderTarget* mRenderTarget = nullptr;
	QShader mFragmentShader;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mShaderBindings;
	QRhiGraphicsPipelineRef mPipeline;

#ifdef QENGINE_WITH_EDITOR
	QShader mOutliningFS;
	QRhiShaderResourceBindingsRef mOutliningBindings;
	QRhiGraphicsPipelineRef mOutliningPipeline;
	QSharedPointer<QDebugUIPainter> mDebugUiPainter;
#endif 
};

#endif // QOutputPassBuilder_h__