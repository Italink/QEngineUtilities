#ifndef QVideoRenderPass_h__
#define QVideoRenderPass_h__

#include "Render/RenderGraph/IRenderPassBuilder.h"
#include <QVideoFrame>
#include <QMutex>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoFrame>
#include <private/qabstractvideobuffer_p.h>

class QENGINECORE_API QVideoPassBuilder : public::IRenderPassBuilder {
	QRP_INPUT_BEGIN(QVideoPassBuilder)
	QRP_INPUT_END()
	QRP_OUTPUT_BEGIN(QVideoPassBuilder)
		QRP_OUTPUT_ATTR(QRhiTextureRef,VideoFrame)
	QRP_OUTPUT_END()
public:
	QVideoPassBuilder();
	void setVideoPath(const QUrl& inPath,bool bAutoPlay = true);
	void play();
	void stop();
public:
	void setup(QRenderGraphBuilder& builder) override;
	void execute(QRhiCommandBuffer* cmdBuffer) override;

	void updateFrame(const QVideoFrame& inFrame);
	void updateTextures(QRhi* rhi, QRhiResourceUpdateBatch* rub);
	void rebuildVideoShader();
private:
	std::unique_ptr<QVideoFrameTextures> mFrameTextures;
	struct RTResource {
		QRhiTextureRef colorAttachment;
		QRhiTextureRenderTargetRef renderTarget;
	};
	RTResource mRT;
	QRhiBufferRef mVertexBuffer;
	QRhiBufferRef mUniformBuffer;
	QRhiSamplerRef mSampler;
	QRhiShaderResourceBindingsRef mBindings;
	QRhiGraphicsPipelineRef mPipeline;

	bool bNeedPlay = false;
	Qt::AspectRatioMode mAspectRatioMode = Qt::AspectRatioMode::KeepAspectRatioByExpanding;
	QScopedPointer<QMediaPlayer> mPlayer;
	QScopedPointer<QVideoSink> mSink;
	QVideoFrame mCurrentFrame;
	QVideoFrameFormat mFormat;

	QShader mVideoVS;
	QShader mVideoFS;

	QRhiSignal mSigUpdateFrameTextures;
	QRhiSignal mSigUploadVertextBuffer;
	QMutex mMutex;
};

#endif // QVideoRenderPass_h__