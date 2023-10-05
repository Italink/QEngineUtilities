#ifndef QPbrMeshPassBuilder_h__
#define QPbrMeshPassBuilder_h__

#include "Render/RenderGraph/PassBuilder/IMeshPassBuilder.h"
#include "QEngineUtilitiesAPI.h"

class QENGINEUTILITIES_API QPbrMeshPassBuilder : public IMeshPassBuilder {
	friend class QRenderGraphBuilder;
public:
	QRP_INPUT_BEGIN(QPbrMeshPassBuilder)
	QRP_INPUT_END()

	QRP_OUTPUT_BEGIN(QPbrMeshPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef, BaseColor)
		QRP_OUTPUT_ATTR(QRhiTextureRef, Position)
		QRP_OUTPUT_ATTR(QRhiTextureRef, Normal)
		QRP_OUTPUT_ATTR(QRhiTextureRef, Metallic)
		QRP_OUTPUT_ATTR(QRhiTextureRef, Roughness)
		QRP_OUTPUT_ATTR(QRhiTextureRef, Depth)
	QRP_OUTPUT_END()
protected:
	void setup(QRenderGraphBuilder& builder) override;
	QRhiTextureRenderTarget* renderTarget() override;
private:
	QRhiTextureRenderTargetRef mRenderTarget;
};

#endif // QPbrMeshPassBuilder_h__
