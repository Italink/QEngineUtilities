#ifndef QSpectrumRenderComponent_h__
#define QSpectrumRenderComponent_h__

#include "Render/Component/QDynamicMeshRenderComponent.h"

class QAudioProvider;
class QSpectrumProvider;

class QSpectrumRenderComponent :public QDynamicMeshRenderComponent {
public:
	QSpectrumRenderComponent();
protected:
	void onRebuildResource() override;
	void onUpdateVertices(QVector<Vertex>& vertices) override;
private:
	QSharedPointer<QAudioProvider> mAudioProvider;
	QSharedPointer<QSpectrumProvider> mSpectruomProvider;
	QVector<float> data;
};

#endif // QSpectrumRenderComponent_h__
