#ifndef QOutliningPassBuilder_h__
#define QOutliningPassBuilder_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
#include "Type/QColor4D.h"

class QENGINECORE_API QOutliningPassBuilder : public IRenderPassBuilder {
	QRP_INPUT_BEGIN(QOutliningPassBuilder)
		QRP_INPUT_ATTR(QRhiTextureRef, BaseColorTexture);
		QRP_INPUT_ATTR(QRhiTextureRef, PositionTexture);
		QRP_INPUT_ATTR(float, MinSeparation) = 1.0f;
		QRP_INPUT_ATTR(float, MaxSeparation) = 3.0f;
		QRP_INPUT_ATTR(float, MinDistance) = 0.5f;
		QRP_INPUT_ATTR(float, MaxDistance) = 2.0f;
		QRP_INPUT_ATTR(int, Radius) = 2;
		QRP_INPUT_ATTR(QColor4D, ColorModifier) = QColor4D(0.324f, 0.063f, 0.099f, 1.0f);
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QOutliningPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, OutliningReslut);
	QRP_OUTPUT_END()
public:
	QOutliningPassBuilder();

public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;
private:
	void refreshParams();

	QShader mOutliningFS;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	struct UniformBlock {
		QGenericMatrix<4, 4, float> VP;
		float MinSeparation;
		float MaxSeparation ;
		float MinDistance;
		float MaxDistance;
		float FarNear;
		int Radius;
		alignas(16) QColor4D ColorModifier;
	}mParams;
	RTResource mRT;
	QRhiBufferRef mUniformBuffer;
	QRhiGraphicsPipelineRef mPipeline;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
};

#endif // QOutliningPassBuilder_h__
