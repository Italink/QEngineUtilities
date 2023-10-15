#ifndef RenderGraphView_h__
#define RenderGraphView_h__

class IRenderer;

#ifdef QENGINE_WITH_EDITOR

#include "GraphEditor.h"
#include "rhi/qrhi.h"

class RenderGraphView : public GraphEditor::Delegate {
public:
	RenderGraphView(IRenderer* renderer);
	void Rebuild();
	void Show();
	void RequestFitScreen();
	QRhiTexture* GetCurrentTexture();
protected:
	void ShowFrameComparer(float thickness, float leftMinWidth, float rightMinWidth, float splitterLongAxisSize = -1.0f);
	bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override { return false; }
	void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected, int slotIndex = -1) override;
	void MoveSelectedNodes(const ImVec2 delta) override {}
	void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override {}
	void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override {}
	void DelLink(GraphEditor::LinkIndex linkIndex) override {}
	void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override;
	const size_t GetTemplateCount() override {return mTemplates.size();}
	const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override;
	const size_t GetNodeCount() override;
	const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override;
	const size_t GetLinkCount() override {
		return mLinks.size();
	}
	const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override;
public:
	IRenderer* mRenderer;
	QList<GraphEditor::Node> mNodes;
	QList<QList<QRhiTexture*>> mNodeTexutes;
	QList<GraphEditor::Link> mLinks;;
	QList<GraphEditor::Template> mTemplates;
	GraphEditor::Options mOptions;
	GraphEditor::ViewState mViewState;
	GraphEditor::FitOnScreen mFitOnScreen = GraphEditor::Fit_AllNodes;
	int mLastNodeIndex = -1;
	int mLastNodeSlotIndex = -1;
	int mCurrentNodeIndex = -1;
	int mCurrentNodeSlotIndex = -1;
	int NodeSpacing = 100;
	bool bShowFrameComparer = false;
	float mCompSplitFactor = 0.5;
	QRhiTexture* mCompLeft = nullptr;
	QRhiTexture* mCompRight = nullptr;

	QList<QRhiTextureRenderTarget*> mLastActivatedRenderTargets;
};

#endif

#endif // RenderGraphView_h__
