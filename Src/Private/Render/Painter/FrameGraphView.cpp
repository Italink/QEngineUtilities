#ifdef QENGINE_WITH_EDITOR

#include "FrameGraphView.h"
#include "Render/IRenderPass.h"
#include "QEngineEditorStyleManager.h"

FrameGraphView::FrameGraphView() {
	mOptions.mRenderGrid = true;
	mOptions.mDisplayLinksAsCurves = true;
	mOptions.mBackgroundColor = ImColor(0, 0, 0, 0);
	mOptions.mGridColor = ImColor(128, 128, 128, 128);
	mOptions.mGridColor2 = ImColor(128, 128, 128, 64);
	mOptions.mSelectedNodeBorderColor = ImColor(0,0,0,0);
	//mOptions.mQuadSelection = ImColor(0, 0, 0, 0);
	//mOptions.mQuadSelectionBorder = ImColor(0, 0, 0, 0);
	//mOptions.mFrameFocus = ImColor(0, 0, 0, 0);
	//mOptions.mDefaultSlotColor = ImColor(15, 100, 200, 255);
	mOptions.mNodeSlotRadius = 0;
	mOptions.mRounding = 0;
	mOptions.mMinZoom = 0.1f;
	mOptions.mMaxZoom = 10.0f;;
}

void FrameGraphView::Rebuild(QFrameGraph* frameGraph) {
	mNodes.clear();
	mNodeTexutes.clear();
	mLinks.clear();
	mTemplates.clear();
	const QList<IRenderPass*>& passes = frameGraph->mRenderPassTopology;
	size_t maxOutSlot = 0;

	/*Create Nodes*/
	QHash<QString, int> passToNodeIndex;
	for (auto& pass : passes) {
		GraphEditor::Node node;
		node.mName = pass->objectName().toLocal8Bit();
		node.mTemplateIndex = pass->getOutputTextureSize();
		node.mRect = ImRect(0, 0, mOptions.mNodeSlotSize, node.mTemplateIndex *(mOptions.mNodeSlotSize));
		node.mSelected = false;
		maxOutSlot = qMax(maxOutSlot, node.mTemplateIndex);
		passToNodeIndex[pass->objectName()] = mNodes.size();
		mNodes << std::move(node);
		mNodeTexutes << std::move(pass->getOutputTextures());
	}
	/*Create Templates*/
	std::vector<std::string> slot;
	for (int i = 0; i <= maxOutSlot; i++) {
		GraphEditor::Template temp;
		temp.mHeaderColor = IM_COL32(200, 200, 200, 255);
		temp.mBackgroundColor = IM_COL32(0, 0, 0, 255);
		temp.mBackgroundColorOver = IM_COL32(20, 20, 20, 255);
		temp.mInputNames = { "in" };
		temp.mInputColors = nullptr;
		temp.mOutputNames = slot;
		temp.mOutputColors = nullptr;
		mTemplates << std::move(temp);
		slot.push_back("out " + std::to_string(i));
	}

	/*Create Links*/
	for (auto& pass : passes) {
		for (auto& inputLink : pass->getInputTextureLinks()) {
			GraphEditor::Link link;
			link.mInputNodeIndex = passToNodeIndex[inputLink.first];
			link.mInputSlotIndex = inputLink.second;
			link.mOutputNodeIndex = passToNodeIndex[pass->objectName()];
			link.mOutputSlotIndex = 0;
			mLinks<<std::move(link);
		}
	}

	GraphEditor::Node node;
	node.mName = "SwapChain";
	node.mTemplateIndex = 1;
	node.mRect = ImRect(0, 0, mOptions.mNodeSlotSize, node.mTemplateIndex * (mOptions.mNodeSlotSize));
	node.mSelected = false;
	mNodes << std::move(node);
	mNodeTexutes.resize(mNodes.size());

	GraphEditor::Link link;
	link.mInputNodeIndex = passToNodeIndex[frameGraph->mOutputSlot.first];
	link.mInputSlotIndex = frameGraph->mOutputSlot.second;
	link.mOutputNodeIndex = mNodes.size() -1;
	link.mOutputSlotIndex = 0;
	mLinks << std::move(link);

	/*Beautify Layout*/
	QVector<int> nodeXOffset(mNodes.size());
	QVector<int> nodeYOffset(mNodes.size());
	for (auto& link : mLinks) {
		if (nodeYOffset[link.mInputNodeIndex] == nodeYOffset[link.mOutputNodeIndex]) {
			for (int k = link.mInputNodeIndex + 1; k < link.mOutputNodeIndex; k++) {
				if (nodeYOffset[k] == nodeYOffset[link.mInputNodeIndex]) {
					nodeYOffset[k]++;
				}
			}
		}
	}
	for (int i = 0; i < mNodes.size(); i++) {
		nodeXOffset[i] = i;
	}
	for (int i = 0; i < mNodes.size(); i++) {
		GraphEditor::Node& node = mNodes[i];
		int xOffset = nodeXOffset[i] * (mOptions.mNodeSlotSize + NodeSpacing);
		int yOffset = nodeYOffset[i] * (mOptions.mNodeSlotSize + mOptions.mNodeSlotSize);
		node.mRect = ImRect(xOffset, yOffset, xOffset + mOptions.mNodeSlotSize, yOffset  + node.mTemplateIndex * (mOptions.mNodeSlotSize));
	}
}

void FrameGraphView::Show() {
	GraphEditor::Show(*this, mOptions, mViewState, true, &mFitOnScreen);
}

void FrameGraphView::RequestFitScreen() {
	mFitOnScreen = GraphEditor::Fit_SelectedNodes;
}

QRhiTexture* FrameGraphView::GetCurrentTexture() {
	if (mCurrentNodeIndex >= 0 && mCurrentNodeIndex < mNodeTexutes.size()) {
		return mNodeTexutes[mCurrentNodeIndex].value(mCurrentNodeSlotIndex, nullptr);
	}
	return nullptr;
}

void FrameGraphView::SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected , int slotIndex) {
	if (mCurrentNodeIndex >= 0) {
		mNodes[mCurrentNodeIndex].mSelected = false;
	}
	if (selected) {
		mNodes[nodeIndex].mSelected = selected;
		mCurrentNodeIndex = nodeIndex;
		mCurrentNodeSlotIndex = slotIndex;
	}
}

void FrameGraphView::CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) {
	const float textureNameFontSize = 12.0f * mViewState.mFactor;
	for (int i = 0; i < mNodeTexutes[nodeIndex].size(); i++) {
		ImVec2 min(rectangle.Min.x, rectangle.Min.y + (mOptions.mNodeSlotSize) * mViewState.mFactor * i);
		ImVec2 max(min.x + mOptions.mNodeSlotSize * mViewState.mFactor, min.y + mOptions.mNodeSlotSize * mViewState.mFactor);
		drawList->AddImage((ImTextureID)mNodeTexutes[nodeIndex][i], min,max);
		ImVec2 textSize = ImGui::CalcTextSize(mNodeTexutes[nodeIndex][i]->name().data());
		float offset = (rectangle.GetWidth() - textSize.x * textureNameFontSize / textSize.y) / 2;
		drawList->AddText(NULL, textureNameFontSize , ImVec2(min.x + offset, min.y), ImGui::GetColorU32(ImGuiCol_Text), mNodeTexutes[nodeIndex][i]->name().data());
		if (nodeIndex == mCurrentNodeIndex && i == mCurrentNodeSlotIndex) {
			drawList->AddRect(min, max, IM_COL32(79, 110, 242, 255), 0, 0, 3);
		}
	}
}

const GraphEditor::Template FrameGraphView::GetTemplate(GraphEditor::TemplateIndex index) {
	return mTemplates[index];
}

const size_t FrameGraphView::GetNodeCount() {
	return mNodes.size();
}

const GraphEditor::Node FrameGraphView::GetNode(GraphEditor::NodeIndex index) {
	return mNodes[index];
}

const GraphEditor::Link FrameGraphView::GetLink(GraphEditor::LinkIndex index) {
	return mLinks[index];
}

#endif