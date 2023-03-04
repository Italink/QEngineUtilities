#include "Render/ISceneRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Utils/MathUtils.h"

QMatrix4x4 ISceneRenderComponent::calculateMatrixMVP() {
	return  getMatrixClipWithCorr() * getMatrixView () * calculateMatrixModel();
}

QMatrix4x4 ISceneRenderComponent::getMatrixView() {
	return sceneRenderPass()->getRenderer()->getCamera()->getMatrixView();
}

QMatrix4x4 ISceneRenderComponent::getMatrixClipWithCorr() {
	return sceneRenderPass()->getRenderer()->getCamera()->getMatrixClipWithCorr(mRhi);
}

QMatrix4x4 ISceneRenderComponent::calculateMatrixModel() {
	QMatrix4x4 S;
	S.translate(QVector3D(7, 8, 9));
	S.scale(QVector3D(4, 5, 6));
	S.rotate(10, QVector3D(1.f, 0.f, 0.f));
	S.rotate(12, QVector3D(0.f, 1.f, 0.f));
	S.rotate(33, QVector3D(0.f, 0.f, 1.f));

	QVector3D trans = MathUtils::getMatTranslate(S);
	QVector3D rotation = MathUtils::getMatRotation(S);
	QVector3D scale = MathUtils::getMatScale3D(S);
	return mTransform;
}

ISceneRenderComponent* ISceneRenderComponent::setTranslate(QVector3D translate) {
	MathUtils::setMatTranslate(mTransform, translate);
	return this;
}

ISceneRenderComponent* ISceneRenderComponent::setRotation(QVector3D rotation) {
	MathUtils::setMatRotation(mTransform, rotation);
	return this;
}

ISceneRenderComponent* ISceneRenderComponent::setScale3D(QVector3D scale3D) {
	MathUtils::setMatScale3D(mTransform, scale3D);
	return this;
}

ISceneRenderComponent* ISceneRenderComponent::setTransform(QMatrix4x4 transform) {
	mTransform = transform;
	return this;
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
