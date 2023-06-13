#ifndef QAudioProvider_h__
#define QAudioProvider_h__

#include <QObject>
#include <QBuffer>
#include <QUrl>
#include <QAudioFormat>
#include <QSharedPointer>
#include <QAudioDevice>
#include "QEngineCoreAPI.h"

class QBuffer;
class QMediaDevices;
class QAudioDecoder;
class QAudioSink;
class QSpectrumProvider;
struct fftw_plan_s;

class QENGINECORE_API QAudioProvider: public QObject {
	Q_OBJECT
public:
	enum WindowFunction {
		NoWindow,
		GuassWindow,
		HannWindow,
		HammingWindow,
		BartlettWindow,
		TriangleWindow,
		BlackmanWindow,
		NuttallWindow,
		SinWindow
	};
	Q_ENUM(WindowFunction)
public:
	QAudioProvider();
	void setSource(QUrl inUrl);
	void setAudioDevice(const QAudioDevice& deviceInfo);
	void setWindowFunction(WindowFunction inType);
	void setFramesPerBuffer(int size);
	void play();

	QUrl getSource() const { return mUrl; }
	WindowFunction getWindowFunction() const { return mCtx.windowType; }
	int getFramesPerBuffer() const { return mCtx.framesPerBuffer; }
	QList<QAudioDevice> getAudioOutputDevices();
	QAudioFormat getAudioFormat();
	const QVector<double>& getFftCache();
	QSharedPointer<QSpectrumProvider> createSpectrumProvider();
protected:
	void tryExecuteFft();
	void rebuildAudioData();
	void rebuildAudioSink();
	void rebuildFftData();
	void rebuildWindowBuffer();
	QByteArray getCurrentAudioData(int inDesiredSize);
private:
	QUrl mUrl;
	QByteArray mAudioData;
	QBuffer mAudioBuffer;
	QSharedPointer<QAudioDecoder> mDecoder;			//音频解码器
	QSharedPointer<QAudioSink> mSink;				//音频输出设备

	QVector<double> mWindow;
	QSharedPointer<fftw_plan_s> mFftPlan;
	QSharedPointer<double> mFftInput;
	QSharedPointer<double> mFftOutput;

	QVector<double> mFftResultCache;

	struct Context {
		QAudioDevice audioDevice;
		QAudioFormat audioFormat;
		WindowFunction windowType;
		int framesPerBuffer;
		int pos = 0;
	};
	Context mDesiredCtx;
	Context mCtx;
};

class QENGINECORE_API QSpectrumProvider {
	friend class QAudioProvider;
public:
	void setBarSize(int size);
	void setLowFreq(int low);
	void setHighFreq(int high);

	int getBarSize() const { return mAmp.size(); }
	int getLowFreq()const { return mLowFreq; }
	int getHighFreq()const { return mHighFreq; }

	const QVector<float>& calculateSpectrum();
protected:
	QSpectrumProvider(QAudioProvider* inProvider);
	void refreshFreq();
private:
	QAudioProvider* mAudioProvider = nullptr;
	int mDesiredSize;
	QVector<float> mFreq;
	QVector<float> mAmp;
	QVector<float> mSmoothCache;
	QVector<float> mSmoothFallCache;
	int mLowFreq = 0;					//	最低频
	int mHighFreq = 20000;				//  最高频
	float mRms = 0.0f;					//	当前频谱的均值
	float mSmoothRangeFactor = 0.5f;	//	前后两帧数据的平滑原子
	float mSmoothRangeLowCache = 0.0f;
	float mSmoothRangeHighCache = 0.0f;
	float mSmoothRiseFactor = 1.0f;		//	单个柱子上升时的平滑因子
	float mSmoothFallFactor = 0.005f;   //  1.0f = no smooth
};


#endif // QAudioProvider_h__
