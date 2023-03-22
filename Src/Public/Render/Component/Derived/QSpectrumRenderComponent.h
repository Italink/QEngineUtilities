#ifndef QSpectrumRenderComponent_h__
#define QSpectrumRenderComponent_h__

#include "Render/Component/QDynamicMeshRenderComponent.h"
#include "Utils/QAudioProvider.h"

class QSpectrumRenderComponent :public QDynamicMeshRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QAudioProvider::WindowFunction WindowFunction READ getWindowFunction WRITE setWindowFunction)
	Q_PROPERTY(int FramesPerBuffer READ getFramesPerBuffer WRITE setFramesPerBuffer)
	Q_PROPERTY(int BarCount READ getBarCount WRITE setBarCount)
	Q_PROPERTY(int LowFreq READ getLowFreq WRITE setLowFreq)
	Q_PROPERTY(int HighFreq READ getHighFreq WRITE setHighFreq)
	Q_META_BEGIN(QSpectrumRenderComponent)
		Q_META_P_NUMBER_LIMITED(FramesPerBuffer,4,14)
		Q_META_P_NUMBER_LIMITED(BarCount, 1, 1000)
		Q_META_P_NUMBER_LIMITED(LowFreq, 0, 48000)
		Q_META_P_NUMBER_LIMITED(HighFreq, 0, 48000)
	Q_META_END()
public:
	QSpectrumRenderComponent();

	QAudioProvider::WindowFunction getWindowFunction() { return mAudioProvider->getWindowFunction(); }
	void setWindowFunction(QAudioProvider::WindowFunction inVar) { mAudioProvider->setWindowFunction(inVar); }

	int getFramesPerBuffer() { return mAudioProvider->getFramesPerBuffer(); }
	void setFramesPerBuffer(int inVar) { mAudioProvider->setFramesPerBuffer(inVar); }

	int getBarCount() { return mSpectruomProvider->getBarSize(); }
	void setBarCount(int inVar) { mSpectruomProvider->setBarSize(inVar); }

	int getLowFreq() { return mSpectruomProvider->getLowFreq(); }
	void setLowFreq(int inVar) { mSpectruomProvider->setLowFreq(inVar); }

	int getHighFreq() { return mSpectruomProvider->getHighFreq(); }
	void setHighFreq(int inVar) { mSpectruomProvider->setHighFreq(inVar); }
protected:
	void onRebuildResource() override;
	void onUpdateVertices(QVector<Vertex>& vertices) override;
private:
	QSharedPointer<QAudioProvider> mAudioProvider;
	QSharedPointer<QSpectrumProvider> mSpectruomProvider;
	QVector<float> data;
};

#endif // QSpectrumRenderComponent_h__
