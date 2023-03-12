#ifndef QVideoRenderPass_h__
#define QVideoRenderPass_h__

#include "Render/IRenderPass.h"
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoFrame>
#include <private/qabstractvideobuffer_p.h>

class QVideoRenderPass : public IRenderPassBase {
	Q_OBJECT
		Q_PROPERTY(QMediaPlayer* MediaPlayer READ getPlayer)
private:
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;

	std::unique_ptr<QVideoFrameTextures> mFrameTextures;
	std::unique_ptr<QRhiBuffer> mVertexBuffer;
	std::unique_ptr<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;

	QUrl mVideoPath;
	Qt::AspectRatioMode mAspectRatioMode = Qt::KeepAspectRatio;
	QScopedPointer<QMediaPlayer> mPlayer;
	QScopedPointer<QVideoSink> mSink;
	QVideoFrame mCurrentFrame;
	QVideoFrameFormat mFormat;

	QRhiEx::Signal sigUpdateFrameTextures;
	QRhiEx::Signal sigUploadVertextBuffer;
public:
	enum OutSlot {
		Output
	};
	QVideoRenderPass();
	QVideoRenderPass* setupVideoPath(const QUrl& inPath);
	QMediaPlayer* getPlayer() { return mPlayer.get(); }
protected:
	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	void updateFrame(const QVideoFrame& inFrame);
	void updateTextures(QRhiResourceUpdateBatch* rub);
	void rebuildGraphicsPipeline();

};
#endif // QVideoRenderPass_h__