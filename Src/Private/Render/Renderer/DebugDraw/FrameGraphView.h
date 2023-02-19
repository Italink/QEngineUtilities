#ifndef FrameGraphView_h__
#define FrameGraphView_h__

#include "GraphEditor.h"
#include "Render/Renderer/QWindowRenderer.h"

struct FrameGraphDelegate : public GraphEditor::Delegate {
	void rebuild(QFrameGraph* frameGraph);
	bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override { return false; }
	void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override {
		if (mCurrentNodeIndex >= 0) {
			mNodes[mCurrentNodeIndex].mSelected = false;
		}
		mNodes[nodeIndex].mSelected = true;
		mCurrentNodeIndex = nodeIndex;
	}
	void MoveSelectedNodes(const ImVec2 delta) override {}
	void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override {}
	void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override {}
	void DelLink(GraphEditor::LinkIndex linkIndex) override {}
	void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override;
	const size_t GetTemplateCount() override {return mTemplates.size();}
	const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override {
		return mTemplates[index];
	}
	const size_t GetNodeCount() override {
		return mNodes.size();
	}
	const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override {
		return mNodes[index];
	}
	const size_t GetLinkCount() override {
		return mLinks.size();
	}
	const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override {
		return mLinks[index];
	}
public:
	QList<GraphEditor::Node> mNodes;
	QList<QList<QRhiTexture*>> mNodeTexutes;
	QList<GraphEditor::Link> mLinks;;
	QList<GraphEditor::Template> mTemplates;
	int mCurrentNodeIndex;
	const int TextureSize = 100;
	const int TextSize = 40;
	const int NodeSpacing = 100;
};


#endif // FrameGraphView_h__
