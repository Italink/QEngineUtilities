#include "QFrameGraph.h"
#include "Render/IRenderer.h"
#include "Render/IRenderPass.h"

QFrameGraphBuilder QFrameGraph::Begin() {
	return QFrameGraphBuilder();
}

void QFrameGraph::compile(IRenderer* inRenderer) {
	mRenderPassTopology.clear();
	QMap<QString, bool> visited;
	while (mRenderPassTopology.size() != mRenderPassNodeMap.size()) {
		for (auto& node : mRenderPassNodeMap) {
			bool bHasDependent = false;
			for (auto depName : node->getDependentPassNodeNames()) {
				if (!visited.contains(depName)) {
					bHasDependent = true;
					break;
				}
			}
			if (!bHasDependent && !visited[node->objectName()]) {
				visited[node->objectName()] = true;
				mRenderPassTopology << node;
			}
		}
	}
	for (auto renderPass : mRenderPassTopology) {
		renderPass->setRenderer(inRenderer);
	}
}

void QFrameGraph::render(QRhiCommandBuffer* inCmdBuffer) {
	for (auto& renderPass : mRenderPassTopology) {
		renderPass->render(inCmdBuffer);
	}
}

void QFrameGraph::resize(const QSize& size) {
	for (auto& renderPass : mRenderPassTopology) {
		renderPass->resizeAndLinkNode(size);
		renderPass->compile();
	}
}

QRhiTexture* QFrameGraph::getOutputTexture() {
	if (mRenderPassTopology.isEmpty())
		return nullptr;
	if (QRhiTexture* out = getOutputTexture(mOutputSlot.first, mOutputSlot.second))
		return out;
	return mRenderPassTopology.last()->getFirstOutputTexture();
}

QRhiTexture* QFrameGraph::getOutputTexture(const QString& inPassName, const QString& inTexName) {
	if (const auto& Node = mRenderPassNodeMap.value(inPassName)) {
		return Node->getOutputTexture(inTexName);
	}
	return nullptr;
}

QRhiTexture* QFrameGraph::getOutputTexture(const QString& inPassName, int inSlot) {
	if (const auto& Node = mRenderPassNodeMap.value(inPassName)) {
		return Node->getOutputTexture(inSlot);
	}
	return nullptr;
}

void QFrameGraph::addPass(IRenderPass* inNode) {
	mRenderPassNodeMap[inNode->objectName()] = inNode;
}

QFrameGraphBuilder::QFrameGraphBuilder() {
	mFrameGraph = QSharedPointer<QFrameGraph>::create();
}


QFrameGraphBuilder& QFrameGraphBuilder::addPass(IRenderPass* inNode) {
	mFrameGraph->addPass(inNode);
	return *this;
}

QSharedPointer<QFrameGraph> QFrameGraphBuilder::End(const QString& inOutputPass, const int& inSlot) {
	mFrameGraph->mOutputSlot = { inOutputPass ,inSlot };
	return mFrameGraph;
}
