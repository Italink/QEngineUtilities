#ifndef QPixelFilterRenderPass_h__
#define QPixelFilterRenderPass_h__

#include "Render/IRenderPass.h"

class QPixelFilterRenderPass : public IRenderPassBase {
	Q_OBJECT
public:
	enum InSlot {
		Src
	};
	enum OutSlot {
		Result
	};
	QPixelFilterRenderPass* setupFilterCode(QByteArray code);
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
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QByteArray mFilterCode;
	QRhiEx::Signal sigRebuild;
};

#endif // QPixelFilterRenderPass_h__