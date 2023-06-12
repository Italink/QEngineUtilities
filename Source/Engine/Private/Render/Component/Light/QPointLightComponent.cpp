#include "Render/Component/Light/QPointLightComponent.h"

QPointLightComponent::QPointLightComponent() {

}

QVector3D QPointLightComponent::getRadiance() {
	return mRadiance;
}

void QPointLightComponent::setRadiance(QVector3D vec) {
	mRadiance = vec;
	sigNeedUpdate.request();
}

float QPointLightComponent::getDistance() {
	return mDistance;
}

void QPointLightComponent::setDistance(float var) {
	mDistance = var;
	sigNeedUpdate.request();
}

