#include "FrameGraphView.h"
#include "Render/IRenderPass.h"

void FrameGraphDelegate::rebuild(QFrameGraph* frameGraph) {
	mNodes.clear();
	mNodeTexutes.clear();
	mLinks.clear();
	mTemplates.clear();
	const QList<IRenderPassBase*>& passes = frameGraph->getRenderPassTopology();
	size_t maxOutSlot = 0;

	/*Create Nodes*/
	QHash<QString, int> passToNodeIndex;
	for (auto& pass : passes) {
		GraphEditor::Node node;
		node.mName = pass->objectName().toLocal8Bit();
		node.mTemplateIndex = pass->getOutputTextures().size();
		node.mRect = ImRect(0, 0, TextureSize, node.mTemplateIndex *(TextureSize + TextSize));
		node.mSelected = false;
		maxOutSlot = qMax(maxOutSlot, node.mTemplateIndex);
		passToNodeIndex[pass->objectName()] = mNodes.size();
		mNodes << std::move(node);
		mNodeTexutes << std::move(pass->getOutputTextures().values());
	}

	/*Create Templates*/
	std::vector<std::string> slot;
	for (int i = 0; i <= maxOutSlot; i++) {
		GraphEditor::Template temp;
		temp.mHeaderColor = IM_COL32(160, 160, 180, 255);
		temp.mBackgroundColor = IM_COL32(100, 100, 140, 255);
		temp.mBackgroundColorOver = IM_COL32(110, 110, 150, 255);
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
			link.mInputNodeIndex = passToNodeIndex[inputLink.passName];
			link.mInputSlotIndex = inputLink.passSlot;
			link.mOutputNodeIndex = passToNodeIndex[pass->objectName()];
			link.mOutputSlotIndex = 0;
			mLinks<<std::move(link);
		}
	}

	/*Beautify Layout*/
	QVector<int> nodeXOffset(mNodes.size());
	QVector<int> nodeYOffset(mNodes.size());
	for (int i = 0; i < mNodes.size(); i++) {
		nodeXOffset[i] = i;
		for (int j = i + 2; j < mNodes.size(); j++) {
			if (nodeYOffset[i] == nodeYOffset[j]) {
				for (int k = i + 1; k < j; k++) {
					if (nodeYOffset[k] == nodeYOffset[i]) {
						nodeYOffset[k]++;
					}
				}
			}
		}
	}

	for (int i = 0; i < mNodes.size(); i++) {
		GraphEditor::Node& node = mNodes[i];
		int xOffset = nodeXOffset[i] * (TextureSize + NodeSpacing);
		int yOffset = nodeYOffset[i] * (TextureSize + TextureSize);
		node.mRect = ImRect(xOffset, yOffset, xOffset + TextureSize, yOffset + node.mTemplateIndex * (TextureSize + TextSize));
	}
}

void FrameGraphDelegate::CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) {
	float scaleFactor = rectangle.GetWidth() / TextureSize;
	for (int i = 0; i < mNodeTexutes[nodeIndex].size(); i++) {
		ImVec2 min(rectangle.Min.x, rectangle.Min.y + (TextureSize + TextSize) * scaleFactor * i);
		ImVec2 max(min.x + TextureSize * scaleFactor, min.y + TextureSize * scaleFactor);
		drawList->AddImage((ImTextureID)mNodeTexutes[nodeIndex][i], min,max);
		drawList->AddText(NULL, 20.0f * scaleFactor, ImVec2(min.x, min.y + TextureSize * scaleFactor), ImGui::GetColorU32(ImGuiCol_Text), mNodeTexutes[nodeIndex][i]->name().data());
	}
}

