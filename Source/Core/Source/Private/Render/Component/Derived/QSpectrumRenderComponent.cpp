#include "Render/Component/Derived/QSpectrumRenderComponent.h"
#include "Utils/QAudioProvider.h"

QSpectrumRenderComponent::QSpectrumRenderComponent() {
	mAudioProvider.reset(new QAudioProvider);
	mSpectruomProvider = mAudioProvider->createSpectrumProvider();
	mTimer = new QTimer(this);
	connect(mTimer, &QTimer::timeout, [this]() {
		QMutexLocker locker(&mMutex);
		mSpectrum = mSpectruomProvider->calculateSpectrum();
	});
	mTimer->setInterval(10);

}

QString QSpectrumRenderComponent::getAudio() {
	return mAudioProvider->getSource().toLocalFile();
}

void QSpectrumRenderComponent::setAudio(QString inAudio, bool bAutoPlay)
{
	mAudioProvider->setSource(QUrl::fromLocalFile(inAudio));
	if (bAutoPlay) {
		play();
	}
}

void QSpectrumRenderComponent::play()
{
	mAudioProvider->play();
	mTimer->start();
}

void QSpectrumRenderComponent::onRebuildResource() {
	QDynamicMeshRenderComponent::onRebuildResource();
}

void QSpectrumRenderComponent::onUpdateVertices(QVector<Vertex>& vertices) {
	QMutexLocker locker(&mMutex);
	vertices.resize(mSpectrum.size() * 6);
	float width = 2.0 / mSpectrum.size();
	float startX = -1;
	for (int i = 0; i < mSpectrum.size(); i++) {
		Vertex a, b, c, d;

		a.position = QVector3D(startX, 0, 0);
		b.position = QVector3D(startX, mSpectrum[i], 0);

		startX += width;
		c.position = QVector3D(startX, 0, 0);
		d.position = QVector3D(startX, mSpectrum[i], 0);

		a.normal = b.normal = c.normal = d.normal = QVector3D(0, 0, 1);
		a.tangent = b.tangent = c.tangent = d.tangent = QVector3D(1, 0, 0);

		int indexOffset = i * 6;
		vertices[indexOffset + 0] = a;
		vertices[indexOffset + 1] = b;
		vertices[indexOffset + 2] = c;
		vertices[indexOffset + 3] = c;
		vertices[indexOffset + 4] = b;
		vertices[indexOffset + 5] = d;
	}
}

