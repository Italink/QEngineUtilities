#include "QOutputPassBuilder.h"

QOutputPassBuilder::QOutputPassBuilder()
{
	mFragmentShader = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
		layout (binding = 0) uniform sampler2D uSamplerColor;
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;
		void main() {
			outFragColor = vec4(texture(uSamplerColor, vUV).rgb,1.0f);
		}
	)");
	Q_ASSERT(mFragmentShader.isValid());

#ifdef QENGINE_WITH_EDITOR
	mOutliningFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 450
		layout (location = 0) in vec2 vUV;
		layout (location = 0) out vec4 outFragColor;

		layout (binding = 0) uniform sampler2D uSelectMask;

		void main() {
			vec2 texOffset = 1.0 / textureSize(uSelectMask, 0);		// gets size of single texel
		
			float maxdiff = 0.f;
			float value = texture(uSelectMask,vUV).r;
				
			maxdiff = max(maxdiff, abs(texture(uSelectMask,vUV+vec2(texOffset.x,0)).r - value));
			maxdiff = max(maxdiff, abs(texture(uSelectMask,vUV-vec2(texOffset.x,0)).r - value));
			maxdiff = max(maxdiff, abs(texture(uSelectMask,vUV+vec2(0,texOffset.y)).r - value));
			maxdiff = max(maxdiff, abs(texture(uSelectMask,vUV-vec2(0,texOffset.y)).r - value));

			const vec4 outliningColor = vec4(1.0,0.8,0.4,1.0);
			outFragColor = outliningColor * maxdiff;
		}
	)");
#endif
}

void QOutputPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupSampler(mSampler, "OutputPassSamper", QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
	builder.setupShaderResourceBindings(mShaderBindings, "OutputPassBindings", {
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mInput._InitialTexture.get() ,mSampler.get())
	});

	QRhiGraphicsPipelineState PSO;
	PSO.shaderResourceBindings = mShaderBindings.get();
	PSO.sampleCount = builder.getMainRenderTarget()->sampleCount();
	PSO.renderPassDesc = builder.getMainRenderTarget()->renderPassDescriptor();
	PSO.shaderStages = {
		QRhiShaderStage(QRhiShaderStage::Vertex, builder.getFullScreenVS()),
		QRhiShaderStage(QRhiShaderStage::Fragment, mFragmentShader)
	};
	builder.setupGraphicsPipeline(mPipeline, "OutputPassPipeline", PSO);

	mRenderer = builder.getRenderer();
	mRenderTarget = builder.getMainRenderTarget();

#ifdef QENGINE_WITH_EDITOR
	QMousePickingPassBuilder::Output mousePickingOut = builder.addPassBuilder<QMousePickingPassBuilder>("MousePickingPass");

	builder.setupShaderResourceBindings(mOutliningBindings, "OutliningBindings", {
		QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage,mousePickingOut.SelectMask.get() ,mSampler.get()),
	});
	QRhiGraphicsPipeline::TargetBlend targetBlends;
	targetBlends.enable = true;
	PSO.targetBlends = { targetBlends };
	PSO.shaderResourceBindings = mOutliningBindings.get();
	PSO.shaderStages = {
		QRhiShaderStage(QRhiShaderStage::Vertex, builder.getFullScreenVS()),
		QRhiShaderStage(QRhiShaderStage::Fragment, mOutliningFS)
	};
	builder.setupGraphicsPipeline(mOutliningPipeline, "OutliningPipeline", PSO);

	if (!mDebugUiPainter) {
		mDebugUiPainter.reset(new QDebugUIPainter(mRenderer));
		QObject::connect(mDebugUiPainter.get(), &QDebugUIPainter::mouseClicked, mousePickingOut.Pass, &QMousePickingPassBuilder::requestPick);
		QObject::connect(mousePickingOut.Pass, &QMousePickingPassBuilder::componentSelected, mRenderer, &IRenderer::setCurrentObject);
	}
	mDebugUiPainter->setup(builder, mRenderTarget);
#endif
}

void QOutputPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	if (mInput._InitialTexture) {
		QRhiResourceUpdateBatch* batch = nullptr;
#ifdef QENGINE_WITH_EDITOR
		batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
		mDebugUiPainter->resourceUpdate(batch, cmdBuffer->rhi());
#endif
		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };
		cmdBuffer->beginPass(mRenderTarget, clearColor, dsClearValue, batch);

		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mShaderBindings.get());
		cmdBuffer->draw(4);

#ifdef QENGINE_WITH_EDITOR
		if (mRenderer->getCurrentObject()) {
			cmdBuffer->setGraphicsPipeline(mOutliningPipeline.get());
			cmdBuffer->setViewport(QRhiViewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height()));
			cmdBuffer->setShaderResources(mOutliningBindings.get());
			cmdBuffer->draw(4);
		}

		mDebugUiPainter->paint(cmdBuffer, mRenderTarget);
#endif

		cmdBuffer->endPass();
	}
}
