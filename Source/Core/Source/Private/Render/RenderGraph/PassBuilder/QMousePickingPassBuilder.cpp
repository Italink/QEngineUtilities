#include "QMousePickingPassBuilder.h"
#include "Render/IRenderer.h"
#include "Render/IRenderComponent.h"
#include "qvulkanfunctions.h"

QMousePickingPassBuilder::QMousePickingPassBuilder()
{
	mSelectMaskFS = QRhiHelper::newShaderFromCode(QShader::FragmentStage, R"(#version 440
		layout (location = 0) out vec4 outFragColor;
		void main() {
			outFragColor = vec4(1);
		}
	)");
}

void QMousePickingPassBuilder::requestPick(QPoint point)
{
	mReadPoint = point;
	mSigPick.request();
}

void QMousePickingPassBuilder::setup(QRenderGraphBuilder& builder)
{
	builder.setupTexture(mOutput.SelectMask, "SelectMask", QRhiTexture::Format::R8, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupTexture(mDSTexture, "DepthStencil", QRhiTexture::Format::D24S8, builder.getMainRenderTarget()->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
	builder.setupRenderTarget(mRenderTarget, "MousePickingPass", QRhiTextureRenderTargetDescription(mOutput.SelectMask.get(), mDSTexture.get()));
	builder.setupBuffer(mReadbackBuffer, "ReadbackBuffer", QRhiBuffer::Static, QRhiBuffer::UsageFlag::StorageBuffer, 1);	

	builder.setupShaderResourceBindings(mSelectMaskBindings, "SelectMaskBindings", {});

	QRhiGraphicsPipelineState PSO;
	PSO.shaderResourceBindings = mSelectMaskBindings.get();
	PSO.sampleCount = mRenderTarget->sampleCount();
	PSO.renderPassDesc = mRenderTarget->renderPassDescriptor();
	PSO.shaderStages = {
		QRhiShaderStage(QRhiShaderStage::Vertex, builder.getFullScreenVS()),
		QRhiShaderStage(QRhiShaderStage::Fragment, mSelectMaskFS)
	};
	PSO.flags = QRhiGraphicsPipeline::Flag::UsesStencilRef;

	PSO.stencilTest = true;
	PSO.stencilFront.compareOp = QRhiGraphicsPipeline::Equal;
	PSO.stencilFront.passOp = QRhiGraphicsPipeline::Replace;
	PSO.stencilFront.depthFailOp = QRhiGraphicsPipeline::Keep;
	PSO.stencilFront.failOp = QRhiGraphicsPipeline::Keep;
	PSO.stencilBack = PSO.stencilFront;
	builder.setupGraphicsPipeline(mSelectMaskPipeline, "SelectMaskPipeline", PSO);

}

void QMousePickingPassBuilder::execute(QRhiCommandBuffer* cmdBuffer)
{
	for (auto& pipeline : mRenderer->getRenderProxies()) {
		if (!pipeline->hasSubPipeline("DebugId")) {
			pipeline->createSubPipeline("DebugId", mRenderTarget.get(), [](QRhiGraphicsPipeline* pipeline) {
				pipeline->setFlags(QRhiGraphicsPipeline::Flag::UsesStencilRef);
				pipeline->setStencilTest(true);
				QRhiGraphicsPipeline::StencilOpState stencil;
				stencil.compareOp = QRhiGraphicsPipeline::Always;
				stencil.depthFailOp = QRhiGraphicsPipeline::Keep;
				stencil.passOp = QRhiGraphicsPipeline::Replace;
				stencil.failOp = QRhiGraphicsPipeline::Keep;
				pipeline->setStencilFront(stencil);
				pipeline->setStencilBack(stencil);
				for (int i = 0; i < pipeline->shaderStageCount(); i++) {
					const QRhiShaderStage* stage = pipeline->shaderStageAt(i);
					if (stage->type() == QRhiShaderStage::Vertex) {
						pipeline->setShaderStages({ *stage });
						break;
					}
				}
			});
		}
	}

	static const quint32 INVALID_INDEX = 255;

	cmdBuffer->beginPass(mRenderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, INVALID_INDEX }, nullptr, QRhiCommandBuffer::BeginPassFlag::ExternalContent);
	QRhiViewport viewport(0, 0, mRenderTarget->pixelSize().width(), mRenderTarget->pixelSize().height());
	auto pipelines = mRenderer->getRenderProxies();
	auto components = mRenderer->getRenderComponents();
	for (int i = 0; i < pipelines.size(); i++) {
		QRhiGraphicsPipeline* employee = pipelines[i]->gerSubPipeline("DebugId");
		cmdBuffer->setGraphicsPipeline(employee);
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		cmdBuffer->setStencilRef(components.indexOf(pipelines[i]->getRenderComponent()));
		pipelines[i]->draw(cmdBuffer);
	}

	qsizetype currentIndex = components.indexOf(mRenderer->getCurrentObject());
	if (currentIndex == -1)
		currentIndex = INVALID_INDEX;

	cmdBuffer->setGraphicsPipeline(mSelectMaskPipeline.get());
	cmdBuffer->setViewport(viewport);
	cmdBuffer->setShaderResources();
	cmdBuffer->setStencilRef(currentIndex);
	cmdBuffer->draw(4);
	cmdBuffer->endPass();

	if (mSigPick.ensure()) {
		makeCopyTextureToBufferCommand(cmdBuffer);
		mReadbackResult.completed = [this, components]() {
			const uchar* p = reinterpret_cast<const uchar*>(mReadbackResult.data.constData());
			IRenderComponent* renderComponent = components.value(*p);
			Q_EMIT componentSelected(renderComponent);
		};
		QRhiResourceUpdateBatch* batch = cmdBuffer->rhi()->nextResourceUpdateBatch();
		batch->readBackBuffer(mReadbackBuffer.get(), 0, 1, &mReadbackResult);
		cmdBuffer->resourceUpdate(batch);
		cmdBuffer->rhi()->finish();
	}
}

void QMousePickingPassBuilder::makeCopyTextureToBufferCommand(QRhiCommandBuffer* cmdBuffer)
{
	if (cmdBuffer->rhi()->backend() == QRhi::Vulkan) {
		cmdBuffer->beginExternal();
		QRhiVulkan* rhi = *(QRhiVulkan**)(cmdBuffer->rhi());
		QVkTexture* texD = QRHI_RES(QVkTexture, mDSTexture.get());
		QVkBuffer* bufferD = QRHI_RES(QVkBuffer, mReadbackBuffer.get());
		QVkCommandBuffer* cbD = QRHI_RES(QVkCommandBuffer, cmdBuffer);
		QVulkanDeviceFunctions* df = rhi->df;

		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = texD->usageState.access;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.image = texD->image;
		barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = -1;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		barrier.pNext = nullptr;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		df->vkCmdPipelineBarrier(cbD->cb, texD->usageState.stage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy copyDesc = {};
		copyDesc.bufferOffset = 0;
		copyDesc.imageSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		copyDesc.imageSubresource.mipLevel = 0;
		copyDesc.imageSubresource.baseArrayLayer = 0;
		copyDesc.imageSubresource.layerCount = 1;
		copyDesc.imageOffset.x = qBound(0, mReadPoint.x(), mDSTexture->pixelSize().width() - 1);
		copyDesc.imageOffset.y = qBound(0, mReadPoint.y(), mDSTexture->pixelSize().height() - 1);
		copyDesc.imageExtent.width = 1;
		copyDesc.imageExtent.height = 1;
		copyDesc.imageExtent.depth = 1;
		df->vkCmdCopyImageToBuffer(cbD->cb, texD->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, bufferD->buffers[0], 1, &copyDesc);

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = texD->usageState.access;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = texD->usageState.layout;
		df->vkCmdPipelineBarrier(cbD->cb, VK_PIPELINE_STAGE_TRANSFER_BIT, texD->usageState.stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		cmdBuffer->endExternal();
	}
}
