#ifndef ILightRenderComponent_h__
#define ILightRenderComponent_h__

#include "Render/ISceneRenderComponent.h"

class QENGINECORE_API ILightRenderComponent: public ISceneRenderComponent {
	Q_OBJECT
public:
	QRhiSignal sigNeedUpdate;

	ILightRenderComponent() {
		sigNeedUpdate.request();
	}
protected:
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {}
};

#endif // ILightRenderComponent_h__
