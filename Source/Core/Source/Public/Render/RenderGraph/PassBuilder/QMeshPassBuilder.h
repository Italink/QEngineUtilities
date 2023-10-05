#ifndef QMeshPassBuilder_h__
#define QMeshPassBuilder_h__

#include "IMeshPassBuilder.h"

class QENGINECORE_API QMeshPassBuilder: public IMeshPassBuilder {
public:
	QRP_INPUT_BEGIN(QMeshPassBuilder)
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QMeshPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, BaseColor)
	QRP_OUTPUT_END()
protected:
	void setup(QRenderGraphBuilder& builder) override;
	QRhiTextureRenderTarget* renderTarget() override;
private:
	QRhiTextureRenderTargetRef mRenderTarget;
	QRhiRenderBufferRef mDepthStencilBuffer;
};

#endif // QMeshPassBuilder_h__
