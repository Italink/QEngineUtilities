#include "Render/Pass/QVideoRenderPass.h"
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


QVideoRenderPass::QVideoRenderPass()
	: mPlayer(new QMediaPlayer)
	, mSink(new QVideoSink)
{
	connect(mSink.get(), &QVideoSink::videoFrameChanged, this, &QVideoRenderPass::updateFrame);
}

static QShader getShader(const QString& name) {
	QFile f(name);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

void QVideoRenderPass::resizeAndLink(const QSize& size, const TextureLinker& linker) {
	mRT.colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.colorAttachment->create();
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget({ mRT.colorAttachment.get() }));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	mSampler->create();
	linker.registerOutputTexture(OutSlot::Output, "Output", mRT.colorAttachment.get());
	mPipeline.reset();
	sigUpdateFrameTextures.request();
	mFormat = {};
}

void QVideoRenderPass::compile() {
	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(g_quad)));
	mVertexBuffer->create();
	sigUploadVertextBuffer.request();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVideoTextureHelper::UniformData)));
	mUniformBuffer->create();

	mBindings.reset(mRhi->newShaderResourceBindings());

	mSink->setRhi(mRhi);
	mPlayer->setSource(mVideoPath);
	mPlayer->setAudioOutput(new QAudioOutput());
	mPlayer->setVideoOutput(mSink.get());
	mPlayer->play();
}

void QVideoRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	if (sigUploadVertextBuffer.receive()) {
		batch->uploadStaticBuffer(mVertexBuffer.get(), g_quad);
	}
	if (sigUpdateFrameTextures.receive()) {
		updateTextures(batch);
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
	if (mFrameTextures) {
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

void QVideoRenderPass::updateFrame(const QVideoFrame& inFrame) {
	mCurrentFrame = inFrame;
	sigUpdateFrameTextures.request();
}

void QVideoRenderPass::updateTextures(QRhiResourceUpdateBatch* rub) {
	// We render a 1x1 black pixel when we don't have a video
	if (!mCurrentFrame.isValid())
		mCurrentFrame = QVideoFrame(new QMemoryVideoBuffer(QByteArray{ 4, 0 }, 4),
		QVideoFrameFormat(QSize(1, 1), QVideoFrameFormat::Format_RGBA8888));

	mFrameTextures = QVideoTextureHelper::createTextures(mCurrentFrame, mRhi, rub, std::move(mFrameTextures));
	if (!mFrameTextures)
		return;

	QRhiShaderResourceBinding bindings[4];
	auto* b = bindings;
	*(b++) = QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage | QRhiShaderResourceBinding::FragmentStage,
		mUniformBuffer.get());

	auto fmt = mCurrentFrame.surfaceFormat();
	auto textureDesc = QVideoTextureHelper::textureDescription(fmt.pixelFormat());

	for (int i = 0; i < textureDesc->nplanes; ++i)
		(*b++) = QRhiShaderResourceBinding::sampledTexture(i + 1, QRhiShaderResourceBinding::FragmentStage,
		mFrameTextures->texture(i), mSampler.get());
	mBindings->setBindings(bindings, b);
	mBindings->create();
	if (fmt != mFormat) {
		mFormat = fmt;
		if (!mPipeline)
			mPipeline.reset(mRhi->newGraphicsPipeline());
		rebuildGraphicsPipeline();
	}
}

void QVideoRenderPass::rebuildGraphicsPipeline() {
	if (!mPipeline)
		return;
	mPipeline->setTopology(QRhiGraphicsPipeline::TriangleStrip);
	QShader vs = getShader(QVideoTextureHelper::vertexShaderFileName(mFormat));
	Q_ASSERT(vs.isValid());
	QShader fs = getShader(QVideoTextureHelper::fragmentShaderFileName(mFormat, QRhiSwapChain::Format::SDR));
	Q_ASSERT(fs.isValid());
	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({
		{ 4 * sizeof(float) }
	});
	inputLayout.setAttributes({
		{ 0, 0, QRhiVertexInputAttribute::Float2, 0 },
		{ 0, 1, QRhiVertexInputAttribute::Float2, 2 * sizeof(float) }
	});
	mPipeline->setVertexInputLayout(inputLayout);
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mPipeline->create();
}

QVideoRenderPass* QVideoRenderPass::setupVideoPath(const QUrl& inPath) {
	mVideoPath = inPath;
	return this;
}