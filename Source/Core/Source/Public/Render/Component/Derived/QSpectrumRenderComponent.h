#ifndef QSpectrumRenderComponent_h__
#define QSpectrumRenderComponent_h__

#include "Render/Component/QDynamicMeshRenderComponent.h"
#include "QTimer"
#include "Utils/QAudioProvider.h"

class QENGINECORE_API QSpectrumRenderComponent :public QDynamicMeshRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QString Audio READ getAudio WRITE setAudio)
	Q_PROPERTY(QAudioProvider::WindowFunction WindowFunction READ getWindowFunction WRITE setWindowFunction)
	Q_PROPERTY(int FramesPerBuffer READ getFramesPerBuffer WRITE setFramesPerBuffer)
	Q_PROPERTY(int BarCount READ getBarCount WRITE setBarCount)
	Q_PROPERTY(int LowFreq READ getLowFreq WRITE setLowFreq)
	Q_PROPERTY(int HighFreq READ getHighFreq WRITE setHighFreq)

	Q_CLASSINFO("FramesPerBuffer", "Min=4,Max=14")
	Q_CLASSINFO("BarCount", "Min=1,Max=1000")
	Q_CLASSINFO("LowFreq", "Min=0,Max=48000")
	Q_CLASSINFO("HighFreq", "Min=0,Max=48000")

	Q_BUILDER_BEGIN_SCENE_RENDER_COMP(QSpectrumRenderComponent)
		Q_BUILDER_ATTRIBUTE(QString, Audio)
		Q_BUILDER_ATTRIBUTE(QAudioProvider::WindowFunction,WindowFunction)
		Q_BUILDER_ATTRIBUTE(int,FramesPerBuffer)
		Q_BUILDER_ATTRIBUTE(int,BarCount)
		Q_BUILDER_ATTRIBUTE(int,LowFreq)
		Q_BUILDER_ATTRIBUTE(int,HighFreq)
	Q_BUILDER_END()
public:
	QSpectrumRenderComponent();

	QString getAudio();
	void setAudio(QString inAudio, bool bAutoPlay = true);

	void play();

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
	QVector<float> mSpectrum;
	QTimer* mTimer;
	QMutex mMutex;
};

#endif // QSpectrumRenderComponent_h__
