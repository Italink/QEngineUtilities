﻿#include "Utils/QRhiCamera.h"
#include "qevent.h"
#include <QApplication>
#include <QDateTime>

QRhiCamera::QRhiCamera(){

}

void QRhiCamera::setupWindow(QWindow* window) {
	mWindow = window;
	if (mWindow) {
		mWindow->installEventFilter(this);
		setAspectRatio(mWindow->width() / (float)mWindow->height());
		calculateCameraDirection();
		calculateViewMatrix();
	}
}

void QRhiCamera::setupRhi(QRhi* rhi)
{
	mRhi = rhi;
}

float QRhiCamera::getYaw()
{
	return 	mRotation.y();
}

float QRhiCamera::getPitch()
{
	return 	mRotation.x();
}

float QRhiCamera::getRoll()
{
	return 	mRotation.z();
}

void QRhiCamera::setYaw(float inVar) {
	mRotation.setY(inVar);
	calculateViewMatrix();
}

void QRhiCamera::setPitch(float inVar) {
	mRotation.setX(inVar);
	calculateViewMatrix();
}

void QRhiCamera::setRoll(float inVar) {
	mRotation.setZ(inVar);
	calculateViewMatrix();
}

void QRhiCamera::setPosition(const QVector3D& newPosition)
{
	mPosition = newPosition;
	calculateViewMatrix();
}

void QRhiCamera::setRotation(const QVector3D& newRotation)
{
	mRotation = newRotation;
	calculateCameraDirection();
	calculateViewMatrix();
}

QVector3D QRhiCamera::getPosition() {
	return mPosition;
}

QVector3D QRhiCamera::getRotation() {
	return mRotation;
}

void QRhiCamera::setFov(float val) {
	mFov = val;
	calculateProjectionMatrix();
}

void QRhiCamera::setAspectRatio(float val)
{
	mAspectRatio = val;
	calculateProjectionMatrix();
}

void QRhiCamera::setNearPlane(float val) {
	mNearPlane = val;
	calculateProjectionMatrix();
}

void QRhiCamera::setFarPlane(float val) {
	mFarPlane = val;
	calculateProjectionMatrix();
}

QMatrix4x4 QRhiCamera::getProjectionMatrixWithCorr() {
	Q_ASSERT(mRhi);
	return mRhi->clipSpaceCorrMatrix() * getProjectionMatrix();
}

QMatrix4x4 QRhiCamera::getProjectionMatrix()
{
	return  mClipMatrix;
}

QMatrix4x4 QRhiCamera::getViewMatrix()
{
	return mViewMatrix;
}

bool QRhiCamera::eventFilter(QObject* watched, QEvent* event)
{
	static QPoint pressedPos;
	if (watched != nullptr && watched == mWindow) {
		switch (event->type())
		{
		case QEvent::Paint:
		case QEvent::Resize:
			setAspectRatio(mWindow->width() / (float)mWindow->height());
			break;
		case QEvent::MouseButtonPress:
			mWindow->setCursor(Qt::BlankCursor);             //隐藏鼠标光标
			pressedPos = QCursor::pos();
			break;
		case QEvent::MouseButtonRelease:
			pressedPos = { 0,0 };
			mWindow->setCursor(Qt::ArrowCursor);             //显示鼠标光标
			break;
		case QEvent::FocusOut:{
			mKeySet.clear();
			pressedPos = { 0,0 };
			mWindow->setCursor(Qt::ArrowCursor);             //显示鼠标光标
			break;
		}
		case QEvent::Wheel: {
			if (qApp->mouseButtons() & Qt::RightButton) {
				QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
				if (wheelEvent->angleDelta().y() > 10) {
					mMoveSpeed = qBound(0.01f, mMoveSpeed + 0.01f, 10.0f);;
				}
				else if (wheelEvent->angleDelta().y() < -10) {
					mMoveSpeed = qBound(0.01f, mMoveSpeed - 0.01f, 10.0f);;
				}
			}
			break;
		}
		case QEvent::MouseMove: {
			if (qApp->mouseButtons() & Qt::RightButton && !pressedPos.isNull()) {
				QPoint currentPos = QCursor::pos();
				float xoffset = pressedPos.x() - currentPos.x();
				float yoffset = currentPos.y() - pressedPos.y();	// 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
				xoffset *= mRotationSpeed;
				yoffset *= mRotationSpeed;
				float rYaw = getYaw() * M_PI / 180 + xoffset;
				float rPitch = getPitch() * M_PI / 180 - yoffset;

				if (rPitch > 1.55f)         //将俯视角限制到[-89°,89°]，89°约等于1.55
					rPitch = 1.55f;
				if (rPitch < -1.55f)
					rPitch = -1.55f;

				QVector3D rotation = mRotation;
				rotation.setY(rYaw / M_PI * 180);
				rotation.setX(rPitch / M_PI * 180);
				setRotation(rotation);
				QCursor::setPos(pressedPos);   //将鼠标移动窗口中央
			}
			break;
		}
		case QEvent::KeyPress: {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			mKeySet.insert(keyEvent->key());
			if (keyEvent->key() == Qt::Key_Escape) {
				mWindow->close();
			}
			break;
		}
		case QEvent::KeyRelease: {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			mKeySet.remove(keyEvent->key());
			break;
		}
		case QEvent::UpdateRequest: {
			int64_t time = QTime::currentTime().msecsSinceStartOfDay();
			if(mLastFrameTimeMs!=0)
				mDeltaTimeMs = time - mLastFrameTimeMs;                           //在此处更新时间差
			mLastFrameTimeMs = time;
			float currentSpeed = mMoveSpeed * mDeltaTimeMs;
			if (!mKeySet.isEmpty()&&qApp->mouseButtons()&Qt::RightButton) {
				QVector3D position = mPosition;
				if (mKeySet.contains(Qt::Key_W))                           //前
					position += currentSpeed * mCameraDirection;
				if (mKeySet.contains(Qt::Key_S))                           //后
					position -= currentSpeed * mCameraDirection;
				if (mKeySet.contains(Qt::Key_A))                           //左
					position -= QVector3D::crossProduct(mCameraDirection, mCameraUp) * currentSpeed;
				if (mKeySet.contains(Qt::Key_D))                           //右
					position += QVector3D::crossProduct(mCameraDirection, mCameraUp) * currentSpeed;
				if (mKeySet.contains(Qt::Key_Space))                       //上浮
					position.setY(position.y() + currentSpeed);
				if (mKeySet.contains(Qt::Key_Shift))                       //下沉
					position.setY(position.y() - currentSpeed);
				setPosition(position);
			}
			break;
		}
		case QEvent::WindowActivate: {
			break;
		}
		case  QEvent::WindowDeactivate: {
			mKeySet.clear();
			break;
		}

		case  QEvent::Close: {
			mKeySet.clear();
			break;
		}
		default:
			break;
		}
	}
	return QObject::eventFilter(watched, event);
}

void QRhiCamera::calculateViewMatrix()
{
	mViewMatrix.setToIdentity();
	mViewMatrix.lookAt(mPosition, mPosition + mCameraDirection, mCameraUp);
}

void QRhiCamera::calculateProjectionMatrix()
{
	mClipMatrix.setToIdentity();
	mClipMatrix.perspective(mFov, mAspectRatio, mNearPlane, mFarPlane);
}

void QRhiCamera::calculateCameraDirection()
{
	float xzLen = cos(getPitch() * M_PI / 180);
	mCameraDirection.setX(xzLen * cos(getYaw() * M_PI / 180));
	mCameraDirection.setY(sin(getPitch() * M_PI / 180));
	mCameraDirection.setZ(xzLen * sin(-getYaw() * M_PI / 180));
	mCameraRight = QVector3D::crossProduct({ 0.0f,-1.0f,0.0f }, mCameraDirection);
	mCameraUp = QVector3D::crossProduct(mCameraRight, mCameraDirection);         //摄像机上向量
}