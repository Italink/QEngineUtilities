#include "IMeshPassBuilder.h"
#include "Render/IRenderer.h"
#include "Render/IRenderComponent.h"

void IMeshPassBuilder::setup(QRenderGraphBuilder& builder)
{
	for (auto& component : mRenderer->getRenderComponents()) {
		if (component->getRhi() != builder.getRhi()) {
			component->initialize(builder.getRenderer(), renderTarget());
		}		
		if (component->mSigRebuildResource.ensure()) {
			component->onRebuildResource();
		}
	}
}


void IMeshPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	for (auto& comp : mRenderer->getRenderComponents())
		comp->onPreRenderTick(cmdBuffer);

	QPrimitiveRenderProxy::UpdateContext context;
	context.projectionMatrix = mRenderer->getCamera()->getProjectionMatrix();
	context.projectionMatrixWithCorr = mRenderer->getCamera()->getProjectionMatrixWithCorr();
	context.viewMatrix = mRenderer->getCamera()->getViewMatrix();

	QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
	for (auto& pipeline : mRenderer->getRenderProxies()) {
		pipeline->tryCreate(renderTarget());
		pipeline->tryUpload(batch);
		pipeline->update(batch, context);
	}

	cmdBuffer->beginPass(renderTarget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch,QRhiCommandBuffer::BeginPassFlag::ExternalContent);
	QRhiViewport viewport(0, 0, renderTarget()->pixelSize().width(), renderTarget()->pixelSize().height());
	for (auto& pipeline : mRenderer->getRenderProxies()) {
		cmdBuffer->setGraphicsPipeline(pipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		pipeline->draw(cmdBuffer);
	}
	cmdBuffer->endPass();
}