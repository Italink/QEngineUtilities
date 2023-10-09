#ifndef QPointLightComponent_h__
#define QPointLightComponent_h__

#include "Render/ILightComponent.h"

class QENGINECORE_API QPointLightComponent : public ILightRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QVector3D Radiance READ getRadiance WRITE setRadiance)
		Q_PROPERTY(float Distance READ getDistance WRITE setDistance)

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QPointLightComponent)
		Q_BUILDER_ATTRIBUTE(QVector3D, Radiance)
		Q_BUILDER_ATTRIBUTE(float, Distance)
	Q_BUILDER_END()
public:
	QPointLightComponent();

	QVector3D getRadiance();
	void setRadiance(QVector3D vec);

	float getDistance();
	void setDistance(float var);

	struct Params {
		QVector3D position;
		uint32_t _padding;
		QVector3D radiance;
		float distance;
	};
private:
	QVector3D mRadiance = QVector3D(100, 100, 100);
	float mDistance = 1000;
};

#endif // QPointLightComponent_h__
