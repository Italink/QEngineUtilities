#include "QVideoPassBuilder.h"
#include <private/qvideotexturehelper_p.h>
#include <private/qmemoryvideobuffer_p.h>
#include <QFile>
#include "QAudioOutput"

static const float g_quad[] = {
	// 4 clockwise rotation of texture vertexes (the second pair)
	// Rotation 0
	-1.f, -1.f,   0.f, 0.f,
	-1.f, 1.f,    0.f, 1.f,
	1.f, -1.f,    1.f, 0.f,
	1.f, 1.f,     1.f, 1.f,
	// Rotation 90
	-1.f, -1.f,   0.f, 1.f,
	-1.f, 1.f,    1.f, 1.f,
	1.f, -1.f,    0.f, 0.f,
	1.f, 1.f,     1.f, 0.f,

	// Rotation 180
	-1.f, -1.f,   1.f, 1.f,
	-1.f, 1.f,    1.f, 0.f,
	1.f, -1.f,    0.f, 1.f,
	1.f, 1.f,     0.f, 0.f,
	// Rotation 270
	-1.f, -1.f,   1.f, 0.f,
	-1.f, 1.f,    0.f, 0.f,
	1.f, -1.f,    1.f, 1.f,
	1.f, 1.f,     0.f, 1.f
};
QVideoPassBuilder::QVideoPassBuilder()
	: mPlayer(new QMediaPlayer)
	, mSink(new QVideoSink)
{
	QObject::connect(mSink.get(), &QVideoSink::videoFrameChanged, [this](const QVideoFrame& frame) {
		this->updateFrame(frame);
	});
	mSigUploadVertextBuffer.request();
}

void QVideoPassBuilder::setVideoPath(const QUrl& inPath, bool bAutoPlay /*= true*/)
{
	mPlayer->setSource(inPath);
	bNeedPlay = bAutoPlay;
}

void QVideoPassBuilder::play()
{
	bNeedPlay = true;
}

void QVideoPassBuilder::stop()
{
	mPlayer->stop();
}

static QShader getShader(const QString& name) {
	QFile f(name);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

void QVideoPassBuilder::setup(QRenderGraphBuilder& builder)
{
	QMutexLocker locker(&mMutex);
	if (!mSink->rhi()) {
		mSink->setRhi(builder.rhi());
		mPlayer->setAudioOutput(new QAudioOutput());
		mPlayer->setVideoOutput(mSink.get());
	}
	if (bNeedPlay) {
		QMetaObject::invokeMethod(mPlayer.get(), &QMediaPlayer::play);
		bNeedPlay = false;
	}

	builder.setupTexture(mRT.colorAttachment, "VideoTexture", QRhiTexture::RGBA32F, builder.mainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRT.renderTarget, "VideoRT", { mRT.colorAttachment.get() });

	builder.setupSampler(mSampler, "ViedeoSampler",
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge);

	builder.setupBuffer(mVertexBuffer, "VideoVertexBuffer", QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(g_quad));
	builder.setupBuffer(mUniformBuffer, "VideoUniformBuffer", QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVideoTextureHelper::UniformData));

	if (!mVideoFS.isValid())
		return;

	auto fmt = mCurrentFrame.surfaceFormat();
	auto textureDesc = QVideoTextureHelper::textureDescription(fmt.pixelFormat());
	QVector<QRhiShaderResourceBinding> bindings;
	bindings << QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage, mUniformBuffer.get());
	for (int i = 0; i < textureDesc->nplanes && mFrameTextures; ++i)
		bindings << QRhiShaderResourceBinding::sampledTexture(i + 1, QRhiShaderResourceBinding::FragmentStage, mFrameTextures->texture(i), mSampler.get());
	builder.setupShaderResourceBindings(mBindings, "VideoBindings", bindings);

	QRhiGraphicsPipelineState PSO;
	QRhiGraphicsPipeline::TargetBlend blendState;
	PSO.topology = QRhiGraphicsPipeline::TriangleStrip;
	PSO.sampleCount = mRT.renderTarget->sampleCount();
	PSO.shaderStages = {
		{ QRhiShaderStage::Vertex, mVideoVS },
		{ QRhiShaderStage::Fragment, mVideoFS }
	};
	PSO.vertexInputLayout.setBindings({
		 4 * sizeof(float)
	});
	PSO.vertexInputLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float2, 0 },
		{ 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) }
	});

	PSO.shaderResourceBindings = mBindings.get();
	PSO.renderPassDesc = mRT.renderTarget->renderPassDescriptor();
	builder.setupGraphicsPipeline(mPipeline, "VideoPipeline", PSO);

	mOutput.VideoFrame = mRT.colorAttachment;
}

void QVideoPassBuilder::execute(QRhiCommandBuffer* cmdBuffer) {
	QMutexLocker locker(&mMutex);
	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	if (mSigUploadVertextBuffer.ensure()) {
		batch->uploadStaticBuffer(mVertexBuffer.get(), g_quad);
	}
	if (mSigUpdateFrameTextures.ensure()) {
		updateTextures(cmdBuffer->rhi(),batch);
	}
	QRect rect(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height());
	int frameRotationIndex = (mCurrentFrame.rotationAngle() / 90) % 4;
	QSize frameSize = mCurrentFrame.size();
	if (frameRotationIndex % 2)
		frameSize.transpose();
	QSize scaled = frameSize.scaled(rect.size(), mAspectRatioMode);
	QRect videoRect = QRect(QPoint(0, 0), scaled);
	videoRect.moveCenter(rect.center());
	float mirrorFrame = mCurrentFrame.mirrored() ? -1.f : 1.f;
	float xscale = mirrorFrame * float(videoRect.width()) / float(rect.width());
	float yscale = 1.f * float(videoRect.height()) / float(rect.height());
	QMatrix4x4 transform;
	transform.scale(xscale, yscale);
	QByteArray uniformData;
	QVideoTextureHelper::updateUniformData(&uniformData, mCurrentFrame.surfaceFormat(), mCurrentFrame, transform, 1.f, 100);
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, uniformData.size(), uniformData.constData());
	cmdBuffer->resourceUpdate(batch);	

	if (mFrameTextures && mVideoFS.isValid()&& mPipeline) {
		cmdBuffer->beginPass(mRT.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, nullptr);
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRT.renderTarget->pixelSize().width(), mRT.renderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mBindings.get());
		quint32 vertexOffset = quint32(sizeof(float)) * 16 * frameRotationIndex;
		const QRhiCommandBuffer::VertexInput vbufBinding(mVertexBuffer.get(), vertexOffset);
		cmdBuffer->setVertexInput(0, 1, &vbufBinding);
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
}


void QVideoPassBuilder::updateFrame(const QVideoFrame& inFrame) {
	QMutexLocker locker(&mMutex);
	mCurrentFrame = inFrame;
	mSigUpdateFrameTextures.request();
}

void QVideoPassBuilder::updateTextures(QRhi* rhi, QRhiResourceUpdateBatch* rub) {
	if (!mCurrentFrame.isValid())
		mCurrentFrame = QVideoFrame(new QMemoryVideoBuffer(QByteArray{ 4, 0 }, 4),
		QVideoFrameFormat(QSize(1, 1), QVideoFrameFormat::Format_RGBA8888));
	mFrameTextures = QVideoTextureHelper::createTextures(mCurrentFrame, rhi, rub, std::move(mFrameTextures));
	if (!mFrameTextures)
		return;
	auto fmt = mCurrentFrame.surfaceFormat();
	if (fmt != mFormat) {
		mFormat = fmt;
		rebuildVideoShader();
	}	
}

void QVideoPassBuilder::rebuildVideoShader() {
	mVideoVS = getShader(QVideoTextureHelper::vertexShaderFileName(mFormat));
	Q_ASSERT(mVideoVS.isValid());
	mVideoFS = getShader(QVideoTextureHelper::fragmentShaderFileName(mFormat, QRhiSwapChain::Format::SDR));
	Q_ASSERT(mVideoFS.isValid());
}
