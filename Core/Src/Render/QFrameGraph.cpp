#include "QFrameGraph.h"
#include "IRenderer.h"
#include "IRenderPass.h"

QRhiTexture* QFrameGraph::getOutputTexture() {
	if (IRenderPassBase* pass = mRenderPassMap.value(mOutput.first)) {
		if (QRhiTexture* texture = pass->getOutputTexture(mOutput.second)) {
			return texture;
		}
	}
	return nullptr;
}

void QFrameGraph::rebuildTopology() {
	mDependMap.clear();
	for (auto& renderPass : mRenderPassMap) {
		for (auto name : renderPass->getDependentRenderPassNames()) {
			mDependMap[renderPass] << mRenderPassMap.value(name);
		}
	}
	mRenderPassTopology.clear();
	while (mRenderPassTopology.size() != mRenderPassMap.size()) {
		for (auto& renderPass : mRenderPassMap) {
			bool bHasDepend = false;
			for (auto& depend : mDependMap[renderPass]) {
				if (!mRenderPassTopology.contains(depend)) {
					bHasDepend = true;
				}
			}
			if (!bHasDepend && !mRenderPassTopology.contains(renderPass)) {
				mRenderPassTopology << renderPass;
				renderPass->cleanupInputLinkerCache();
			}
		}
	}
}

QFrameGraphBuilder::QFrameGraphBuilder() {
	mFrameGraph = QSharedPointer<QFrameGraph>::create();
}

QFrameGraphBuilder* QFrameGraphBuilder::begin() {
	return new QFrameGraphBuilder;
}

QFrameGraphBuilder* QFrameGraphBuilder::addPass(const QString& inName, IRenderPassBase* inRenderPass) {
	inRenderPass->setObjectName(inName);
	mFrameGraph->mRenderPassMap[inRenderPass->objectName()] = inRenderPass;
	return this;
}

QSharedPointer<QFrameGraph> QFrameGraphBuilder::end(const QString& outPass, const int& outPassSlot) {
	QSharedPointer<QFrameGraph> frameGraph = mFrameGraph;
	frameGraph->mOutput = { outPass,outPassSlot };
	delete this;
	return frameGraph;
}
