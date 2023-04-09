#include "QAudioProvider.h"
#include <QMediaDevices>
#include <QAudioDecoder>
#include <QAudioSink>
#include <QEventLoop>
#include <QTimer>
#include <fftw3.h>
#include "DspCurves.h"
#include <iostream>
#include <string>
#include <thread>

QAudioProvider::QAudioProvider()
	: mDecoder(new QAudioDecoder)
{
	mAudioBuffer.setBuffer(&mAudioData);
	setWindowFunction(HannWindow);
	setAudioDevice(QMediaDevices::defaultAudioOutput());
	setFramesPerBuffer(12);
}

void QAudioProvider::setSource(QUrl inUrl) {
	mUrl = inUrl;
	rebuildAudioData();
}

void QAudioProvider::setAudioDevice(const QAudioDevice& deviceInfo) {
	mDesiredCtx.audioDevice = deviceInfo;
}

void QAudioProvider::play() {
	if (mAudioBuffer.isOpen())
		mAudioBuffer.close();
	mAudioBuffer.open(QIODevice::ReadOnly);
	mSink->start(&mAudioBuffer);
}

void QAudioProvider::setWindowFunction(WindowFunction inType) {
	mDesiredCtx.windowType = inType;
}

void QAudioProvider::setFramesPerBuffer(int size) {
	mDesiredCtx.framesPerBuffer = qBound(4, size, 14);
}

QList<QAudioDevice> QAudioProvider::getAudioOutputDevices() {
	return QMediaDevices::audioOutputs();
}

QAudioFormat QAudioProvider::getAudioFormat() {
	return mCtx.audioFormat;
}

const QVector<double>& QAudioProvider::getFftCache() {
	tryExecuteFft();
	return mFftResultCache;
}

QByteArray QAudioProvider::getCurrentAudioData(int inDesiredSize) {
	int fftSize = 1 << mCtx.framesPerBuffer;
	int bytesPerSample = mCtx.audioFormat.bytesPerSample();
	int sampleRate = mCtx.audioFormat.sampleRate();
	return mAudioBuffer.peek(fftSize * mCtx.audioFormat.channelCount() * bytesPerSample);
}

QSharedPointer<QSpectrumProvider> QAudioProvider::createSpectrumProvider() {
	return QSharedPointer<QSpectrumProvider>( new QSpectrumProvider(this));
}

void QAudioProvider::rebuildAudioData() {	
	if (mDesiredCtx.audioDevice != mCtx.audioDevice || mDesiredCtx.audioFormat != mCtx.audioFormat) {
		mCtx.audioDevice = mDesiredCtx.audioDevice;
		mCtx.audioFormat = mDesiredCtx.audioFormat;
		rebuildAudioSink();
	}
	mDecoder->setSource(mUrl);		//设置音频源
	if (mAudioBuffer.isOpen())
		mAudioBuffer.close();
	mAudioBuffer.open(QIODevice::WriteOnly);
	QEventLoop loop;				//使用事件循环等待解码器完全解析完成，如果是长期播放则需要考虑流送
	QObject::connect(mDecoder.get(), &QAudioDecoder::finished, &loop, &QEventLoop::quit);
	QObject::connect(mDecoder.get(), &QAudioDecoder::bufferReady, [this]() {
		const QAudioBuffer& frame = mDecoder->read();
		mAudioBuffer.write(frame.constData<char>(), frame.byteCount());		//向IO设备中写入音频数据
	});
	mDecoder->start();				//开始解码音频
	qDebug() << mDecoder->errorString();
	loop.exec();					//等待事件循环结束
	mAudioBuffer.close();			//关闭IO设备
	mDesiredCtx.pos = 0;
}

void QAudioProvider::rebuildAudioSink() {
	if (!mCtx.audioFormat.isValid()) {
		mCtx.audioFormat = mDesiredCtx.audioFormat = mCtx.audioDevice.preferredFormat();
	}
	bool bActived = mSink ? mSink->state() == QAudio::State::ActiveState : false;
	if (bActived) {
		mSink->stop();
	}
	mSink.reset(new QAudioSink(mCtx.audioDevice, mCtx.audioFormat));
	mDecoder->setAudioFormat(mCtx.audioFormat);
	if (bActived) {
		if (mAudioBuffer.isOpen())
			mAudioBuffer.close();
		mAudioBuffer.open(QIODevice::ReadOnly);
		mSink->start(&mAudioBuffer);
	}
}

void QAudioProvider::rebuildWindowBuffer() {
	size_t size = mWindow.size();
	for (int n = 0; n < size; ++n) {
		float x = 1.0;
		switch (mCtx.windowType) {
		case WindowFunction::NoWindow:
			x = 1.0;
			break;
		case WindowFunction::GuassWindow: {
			float tmp = (n - (size - 1) / 2.0) / (0.4 * (size - 1) / 2);
			x = exp(-0.5 * (tmp * tmp));
			break;
		}
		case WindowFunction::HannWindow:
			x = 0.5 * (1 - cos((2 * M_PI * n) / (size - 1)));
			break;
		case WindowFunction::HammingWindow:
			x = 0.53836 - 0.46164 * cos(2 * M_PI * n / (size - 1));
			break;
		case WindowFunction::BartlettWindow:
			x = 2.0 / (size - 1) * ((size - 1) / 2.0 - abs(n - (size - 1) / 2.0));
			break;
		case WindowFunction::TriangleWindow:
			x = 2.0 / size * (size / 2.0 - abs(n - (size - 1) / 2.0));
			break;
		case WindowFunction::BlackmanWindow:
			x = 0.42 - 0.5 * cos(2 * M_PI * n / (size - 1)) + 0.08 * cos(4 * M_PI * n / (size - 1));
			break;
		case WindowFunction::NuttallWindow:
			x = 0.355768 - 0.487396 * cos(2 * M_PI * n / (size - 1)) + 0.1444232 * cos(4 * M_PI * n / (size - 1)) + 0.012604 * cos(6 * M_PI * n / (size - 1));
			break;
		case WindowFunction::SinWindow:
			x = sin(M_PI * n / (size - 1));
			break;
		}
		mWindow[n] = x;
	}
}

void QAudioProvider::rebuildFftData() {
	int fftSize = 1 << mCtx.framesPerBuffer;
	mFftInput.reset((double*)fftw_malloc(sizeof(double) * fftSize), [](double* data) {
		fftw_free(data);
	});
	mFftOutput.reset((double*)fftw_malloc(sizeof(double) * fftSize), [](double* data) {
		fftw_free(data);
	});
	mFftPlan.reset(fftw_plan_r2r_1d(fftSize, mFftInput.get(), mFftOutput.get(), fftw_r2r_kind::FFTW_R2HC, FFTW_ESTIMATE));
	mWindow.resize(fftSize);
}

void QAudioProvider::tryExecuteFft() {
	if (mDesiredCtx.audioDevice != mCtx.audioDevice || mDesiredCtx.audioFormat != mCtx.audioFormat) {
		mCtx.audioDevice = mDesiredCtx.audioDevice;
		mCtx.audioFormat = mDesiredCtx.audioFormat;
		rebuildAudioSink();
	}
	bool bNeedRebuildWindowBuffer = false;
	if (mDesiredCtx.framesPerBuffer != mCtx.framesPerBuffer) {
		mCtx.framesPerBuffer = mDesiredCtx.framesPerBuffer;
		bNeedRebuildWindowBuffer = true;
		rebuildFftData();
	}
	if (mDesiredCtx.windowType != mCtx.windowType) {
		mCtx.windowType = mDesiredCtx.windowType;
		bNeedRebuildWindowBuffer = true;
	}
	if (bNeedRebuildWindowBuffer) {
		rebuildWindowBuffer();
	}
	int fftSize = 1 << mCtx.framesPerBuffer;
	int bytesPerSample = mCtx.audioFormat.bytesPerSample();
	int sampleRate = mCtx.audioFormat.sampleRate();
	const qint64 spectrumLength = fftSize * mCtx.audioFormat.channelCount() * bytesPerSample;
	const qint64 playPosition = mCtx.audioFormat.bytesForDuration(mSink->elapsedUSecs());
	const qint64 spectrumPosition = playPosition - spectrumLength;
	mDesiredCtx.pos = spectrumPosition;
	if (bNeedRebuildWindowBuffer || mDesiredCtx.pos != mCtx.pos) {
		mCtx.pos = mDesiredCtx.pos;
		QByteArray audioData = mAudioData.sliced(qMax(0, spectrumPosition), spectrumLength);
		QAudioBuffer audioFrame(audioData, mCtx.audioFormat);
		QAudioBuffer::F32S* rawData = audioFrame.data<QAudioBuffer::F32S>();
		int maxFrameCount = qMin(fftSize, audioFrame.frameCount());
		for (int i = 0; i < maxFrameCount; i++) {
			mFftInput.data()[i] = rawData[i].value(QAudioFormat::AudioChannelPosition::FrontRight) * mWindow[i];
		}
		memset(mFftInput.data() + maxFrameCount, 0, fftSize - maxFrameCount);
		fftw_execute(mFftPlan.get());

		mFftResultCache.resize(fftSize / 2);
		for (int i = 0; i < fftSize / 2; i++) {
			const double& real = mFftOutput.data()[i];
			const double& imag = mFftOutput.data()[fftSize - 1 - (fftSize / 2 + i)];
			const double& magnitude = sqrt(real * real + imag * imag);
			const double& freq = DspCurves::freqd(i, fftSize / 2, sampleRate);
			const double& kernel = DspCurves::myAWeight(freq);
			mFftResultCache[i] = magnitude * kernel;
		}
	}
}

QSpectrumProvider::QSpectrumProvider(QAudioProvider* inProvider)
	:mAudioProvider(inProvider)
{
	setBarSize(100);
}

void QSpectrumProvider::setBarSize(int size) {
	mDesiredSize = size;
}

void QSpectrumProvider::setLowFreq(int low) {
	mLowFreq = low;
	refreshFreq();
}

void QSpectrumProvider::setHighFreq(int high) {
	mHighFreq = high;
	refreshFreq();
}

const QVector<float>& QSpectrumProvider::calculateSpectrum() {
	if (mDesiredSize != mAmp.size()) {
		mFreq.resize(mDesiredSize);
		mAmp.resize(mDesiredSize);
		mSmoothCache.resize(mDesiredSize);
		mSmoothFallCache.resize(mDesiredSize);
		refreshFreq();
	}
	const QVector<double>& Fft = mAudioProvider->getFftCache();

	int sampleRate = mAudioProvider->getAudioFormat().sampleRate();
	double df = sampleRate / Fft.size() * 2;
	double valMul = (2.0 / sampleRate) * 200;
	int fftBinIndex = 0;
	int barIndex = 0;
	float freqLast = 0.0f;
	float freqMultiplier;

	mAmp.fill(0);
	while (fftBinIndex < Fft.size() && barIndex < mAmp.size()) {
		float freqLin = ((float)fftBinIndex + 0.5f) * df;
		float freqLog = mFreq[barIndex];
		int fftBinI = fftBinIndex;
		int barI = barIndex;
		if (freqLin <= freqLog) {
			freqMultiplier = (freqLin - freqLast);
			freqLast = freqLin;
			fftBinIndex += 1;
		}
		else {
			freqMultiplier = (freqLog - freqLast);
			freqLast = freqLog;
			barIndex += 1;
		}
		mAmp[barI] += freqMultiplier * Fft[fftBinI] * valMul;
	}

	float rangeLow = 9999990.0f;
	float rangeHigh = -9999990.0f;
	mAmp[0] = qMin(mAmp[0], mAmp[1]);
	mAmp.back() = qMin(mAmp.back(), mAmp[mAmp.size() - 2]);
	for (int i = 0; i < mAmp.size(); i++) {
		mAmp[i] = mAmp[i] * 2.0f;//(float) melSpectrumData[i];
		rangeLow = qMin(mAmp[i], rangeLow);
		rangeHigh = qMax(mAmp[i], rangeHigh);
	}

	const float rangeMax = 1000.0f;
	rangeLow = qBound(-rangeMax, rangeLow, rangeMax);
	rangeHigh = qBound(-rangeMax, rangeHigh, rangeMax);

	mSmoothRangeLowCache = (mSmoothRangeLowCache * (1.0f - mSmoothRangeFactor)) + (rangeLow * mSmoothRangeFactor);
	mSmoothRangeHighCache = (mSmoothRangeHighCache * (1.0f - mSmoothRangeFactor)) + (rangeHigh * mSmoothRangeFactor);
	const double rangeTarget = 0.8f;
	float rangeMul = mSmoothRangeHighCache - mSmoothRangeLowCache;
	if (rangeMul < 1.0f)
		rangeMul = 1.0f;
	rangeMul = rangeTarget / rangeMul;
	for (int i = 0; i < mAmp.size(); i++) {
		double val = mAmp[i];
		val = (val - mSmoothRangeLowCache) * rangeMul;
		if (val >= mSmoothCache[i]) {
			mSmoothCache[i] = (mSmoothCache[i] * (1.0f - mSmoothRiseFactor)) + (val * mSmoothRiseFactor);
			mSmoothFallCache[i] = 0;
		}
		else if (val > 0) {
			mSmoothFallCache[i] = mSmoothFallCache[i] * 2 + mSmoothFallFactor;
			mSmoothCache[i] -= mSmoothFallCache[i];
		}
		mAmp[i] = qBound(0.0, mSmoothCache[i], 1.0);
	}
	return mAmp;
}

void QSpectrumProvider::refreshFreq() {
	if (mLowFreq > mHighFreq) {
		qSwap(mLowFreq, mHighFreq);
	}
	float freqScaleOff = 800;
	float step = (float)((log(mHighFreq / (mLowFreq + (freqScaleOff))) / mFreq.size()) / log(2.0));
	mFreq[0] = mLowFreq + freqScaleOff;
	float stepMul = (float)pow(2.0, step);
	for (int i = 1; i < mFreq.size(); ++i) {
		mFreq[i] = ((mFreq[i - 1] * stepMul * 1.0f));
		mFreq[i - 1] += -freqScaleOff;
	}
}