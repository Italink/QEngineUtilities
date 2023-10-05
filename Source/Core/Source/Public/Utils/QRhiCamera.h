﻿#ifndef QRhiCamera_h__
#define QRhiCamera_h__

#include <QWindow>
#include "QQuaternion"
#include "QMatrix4x4"
#include "Render/RHI/QRhiHelper.h"

class QENGINECORE_API QRhiCamera: public QObject {
	Q_OBJECT
		Q_PROPERTY(QVector3D Position READ getPosition WRITE setPosition)
		Q_PROPERTY(QVector3D Rotation READ getRotation WRITE setRotation)
		Q_PROPERTY(float FOV READ getFov WRITE setFov)
		Q_PROPERTY(float NearPlane READ getNearPlane WRITE setNearPlane)
		Q_PROPERTY(float FarPlane READ getFarPlane WRITE setFarPlane)
		Q_PROPERTY(float MoveSpeed READ getMoveSpeed WRITE setMoveSpeed)
		Q_PROPERTY(float RotationSpeed READ getRotationSpeed WRITE setRotationSpeed)
public:
	QRhiCamera();

	void setupWindow(QWindow* window);
	void setupRhi(QRhi* rhi);

	float getYaw();
	float getPitch();
	float getRoll();

	void setYaw(float inVar);
	void setPitch(float inVar);
	void setRoll(float inVar);

	QVector3D getPosition();
	void setPosition(const QVector3D& newPosition);

	void setRotation(const QVector3D& newRotation);
	QVector3D getRotation();

	float getRotationSpeed() const { return mRotationSpeed; }
	void setRotationSpeed(float val) { mRotationSpeed = val; }

	float getMoveSpeed() const{ return mMoveSpeed; }
	void setMoveSpeed(float val) { mMoveSpeed = val; }
	float& getMoveSpeedRef() { return mMoveSpeed; }

	void setFov(float val);
	float getFov() { return mFov; }

	void setAspectRatio(float val);
	float getAspectRatio() { return mAspectRatio; }

	void setNearPlane(float val);
	float getNearPlane() { return mNearPlane; }

	void setFarPlane(float val);
	float getFarPlane() { return mFarPlane; }

	QMatrix4x4 getProjectionMatrixWithCorr();
	QMatrix4x4 getProjectionMatrix();
	QMatrix4x4 getViewMatrix();
private:
	void calculateViewMatrix();
	void calculateProjectionMatrix();
	void calculateCameraDirection();
	bool eventFilter(QObject* watched, QEvent* event) override;
protected:
	QWindow* mWindow;
	QRhi* mRhi;
	QVector3D mPosition = QVector3D(0, 0, 2);
	QVector3D mRotation = QVector3D(0, 90 , 0);
private:
	QMatrix4x4 mViewMatrix;
	QMatrix4x4 mClipMatrix;

	float mFov = 45.0f;
	float mAspectRatio = 1.0;
	float mNearPlane = 0.1f;
	float mFarPlane = 3000.0f;

	QVector3D mCameraDirection;
	QVector3D mCameraUp;
	QVector3D mCameraRight;

	QSet<int> mKeySet;					     //记录当前被按下按键的集合
	int64_t mDeltaTimeMs = 0;				 //当前帧与上一帧的时间差
	int64_t mLastFrameTimeMs = 0;			 //上一帧的时间

	float mRotationSpeed = 0.003f;			 //鼠标灵敏度
	float mMoveSpeed = 0.1f;				 //控制移动速度
};

#endif // QRhiCamera_h__
