//#ifndef QBlinnPhongLightingPass_h__
//#define QBlinnPhongLightingPass_h__
//
//#include "Render/IRenderPass.h"
//#include "Render/Painter/TexturePainter.h"
//#include "Render/Component/Light/QPointLightComponent.h"
//#include "Render/Component/Light/QDirectionLightComponent.h"
//#include "QEngineUtilitiesAPI.h"
//#include "Type/QColor4D.h"
//
//class QENGINEUTILITIES_API QBlinnPhongLightingPass : public IRenderPass {
//	Q_OBJECT
//		Q_PROPERTY(QColor4D Ambient READ getAmbientLight WRITE setAmbientLight)
//
//		Q_BUILDER_BEGIN_RENDER_PASS(QBlinnPhongLightingPass, Albedo, Position, Normal,Specular, SkyTexture)
//		Q_BUILDER_END_RENDER_PASS(FragColor)
//public:
//	QColor4D getAmbientLight() const;
//	void setAmbientLight(QColor4D val);
//protected:
//	QRhiSignal sigUpdateParams;
//
//	void resizeAndLinkNode(const QSize& size) override;
//	void compile() override;
//	void render(QRhiCommandBuffer* cmdBuffer) override;
//
//	struct RTResource {
//		QScopedPointer<QRhiTexture> colorAttachment;
//		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
//		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
//	};
//	RTResource mRT;
//	QScopedPointer<TexturePainter> mSkyTexturePainter;
//
//	struct Params {
//		QVector3D ambientLight = QVector3D(1, 1, 1);
//		int numOfPointLight;
//		QPointLightComponent::Params pointLights[64];
//
//		int numOfdirectionLight;
//		int _padding[3];
//		QDirectionLightComponent::Params directionLights[64];
//	}mParams;
//	QScopedPointer<QRhiBuffer> mUniformBlock;
//	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
//	QScopedPointer<QRhiSampler> mSampler;
//	QScopedPointer<QRhiShaderResourceBindings> mBindings;
//};
//
//
//#endif // QBlinnPhongLightingPass_h__
