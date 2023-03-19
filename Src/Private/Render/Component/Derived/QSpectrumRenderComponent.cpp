#include "Render/Component/Derived/QSpectrumRenderComponent.h"
#include "Utils/QAudioProvider.h"
#include "QTimer"


QSpectrumRenderComponent::QSpectrumRenderComponent(){
	mAudioProvider.reset(new QAudioProvider);
	mSpectruomProvider = mAudioProvider->createSpectrumProvider();
}

void QSpectrumRenderComponent::onRebuildResource() {
	QDynamicMeshRenderComponent::onRebuildResource();
	mAudioProvider->setSource(QUrl::fromLocalFile("C:/Users/Administrator/Music/C.mp3"));
	mAudioProvider->play();

	QTimer* timer = new QTimer;
	connect(timer, &QTimer::timeout, [this]() {
		data = mSpectruomProvider->calculateSpectrum();
	});
	timer->setInterval(10);
	timer->start();
}

void QSpectrumRenderComponent::onUpdateVertices(QVector<Vertex>& vertices) {
	vertices.resize(data.size() * 6);
	float width = 2.0 / data.size();
	float startX = -1;
	for (int i = 0; i < data.size(); i++) {
		Vertex a, b, c, d;

		a.position = QVector3D(startX, 0, 0);
		b.position = QVector3D(startX, data[i], 0);

		startX += width;
		c.position = QVector3D(startX, 0, 0);
		d.position = QVector3D(startX, data[i], 0);

	
		int indexOffset = i * 6;
		vertices[indexOffset + 0] = a;
		vertices[indexOffset + 1] = b;
		vertices[indexOffset + 2] = c;
		vertices[indexOffset + 3] = c;
		vertices[indexOffset + 4] = b;
		vertices[indexOffset + 5] = d;
	}
}

