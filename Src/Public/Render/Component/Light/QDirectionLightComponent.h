#ifndef QDirectionLightComponent_h__
#define QDirectionLightComponent_h__

#include "Render/ILightComponent.h"

class QDirectionLightComponent : public ILightRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QVector3D Radiance READ getRadiance WRITE setRadiance)
		Q_PROPERTY(QVector3D Direction READ getDirection WRITE setDirection)

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QDirectionLightComponent)
		Q_BUILDER_ATTRIBUTE(QVector3D, Radiance)
		Q_BUILDER_ATTRIBUTE(QVector3D, Direction)
	Q_BUILDER_END()
public:
	QDirectionLightComponent();

	QVector3D getRadiance();
	void setRadiance(QVector3D vec);

	QVector3D getDirection();
	void setDirection(QVector3D vec);

	struct Params {
		QVector3D radiance;
		uint32_t _padding;
		QVector3D direction;
		uint32_t __padding;
	};
private:
	QVector3D mRadiance = QVector3D(100, 100, 100);
	QVector3D mDirection = QVector3D(1, 0, 1);
};

#endif // QDirectionLightComponent_h__