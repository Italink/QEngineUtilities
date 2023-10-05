#ifndef ISceneRenderComponent_h__
#define ISceneRenderComponent_h__

#include "Render/IRenderComponent.h"

#define Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QtClass) \
	struct __Builder{ \
		friend class QtClass; \
		using ClassType = QtClass; \
		 __Builder& setTranslate(QVector3D translate) { mObject->setTranslate(translate); return *this; } \
		 __Builder& setRotation(QVector3D rotation) { mObject->setRotation(rotation); return *this; } \
		 __Builder& setScale3D(QVector3D scale3D) { mObject->setScale3D(scale3D); return *this; } \
		 __Builder& setTransform(QMatrix4x4 transform) { mObject->setTransform(transform); return *this; }

class QENGINECORE_API ISceneRenderComponent: public IRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QMatrix4x4 Transform READ getModelMatrix WRITE setTransform)
public:
	QMatrix4x4 getModelMatrix();

	void setTranslate(QVector3D translate);
	void setRotation(QVector3D rotation);
	void setScale3D(QVector3D scale3D);
	void setTransform(QMatrix4x4 transform);

	QVector3D getTranslate();
	QVector3D getRotation();
	QVector3D getScale3D();
protected:
	QMatrix4x4 mTransform;
};

#endif // ISceneRenderComponent_h__
