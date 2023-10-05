#include "Render/ISceneRenderComponent.h"
#include "Utils/MathUtils.h"
#include "QRhiCamera.h"

QMatrix4x4 ISceneRenderComponent::getModelMatrix() {
	return mTransform;
}

void ISceneRenderComponent::setTranslate(QVector3D translate) {
	MathUtils::setMatTranslate(mTransform, translate);
}

void ISceneRenderComponent::setRotation(QVector3D rotation) {
	MathUtils::setMatRotation(mTransform, rotation);
}

void ISceneRenderComponent::setScale3D(QVector3D scale3D) {
	MathUtils::setMatScale3D(mTransform, scale3D);
}

void ISceneRenderComponent::setTransform(QMatrix4x4 transform) {
	mTransform = transform;
}

QVector3D ISceneRenderComponent::getTranslate() {
	return MathUtils::getMatTranslate(mTransform);
}

QVector3D ISceneRenderComponent::getRotation() {
	return MathUtils::getMatRotation(mTransform);
}

QVector3D ISceneRenderComponent::getScale3D() {
	return MathUtils::getMatScale3D(mTransform);
}
