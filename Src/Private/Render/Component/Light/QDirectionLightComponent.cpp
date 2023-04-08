#include "Render/Component/Light/QDirectionLightComponent.h"

QDirectionLightComponent::QDirectionLightComponent() {

}

QVector3D QDirectionLightComponent::getRadiance() {
	return mRadiance;
}

void QDirectionLightComponent::setRadiance(QVector3D vec) {
	mRadiance = vec;
}

QVector3D QDirectionLightComponent::getDirection() {
	return mDirection;
}

void QDirectionLightComponent::setDirection(QVector3D vec) {
	mDirection = vec;
}

