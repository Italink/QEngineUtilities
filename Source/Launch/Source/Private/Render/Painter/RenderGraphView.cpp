#ifdef QENGINE_WITH_EDITOR

#include "RenderGraphView.h"
#include "Render/IRenderer.h"
#include "QEngineEditorStyleManager.h"
#include <QApplication>
#include "Render/RenderGraph/QRenderGraphBuilder.h"

RenderGraphView::RenderGraphView(IRenderer* renderer)
	: mRenderer(renderer)
{
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
	mOptions.mMaxZoom = 10.0f;

}

void RenderGraphView::Show() {
	Rebuild();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (bShowFrameComparer) {
		ShowFrameComparer(1, 2, 2);
	}
	else {
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y));
		ImGui::Begin("Frame Graph", NULL, 
			ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoNav
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_UnsavedDocument);
		GraphEditor::Show(*this, mOptions, mViewState, true, &mFitOnScreen);
		ImGui::End();
	}
	ImGui::PopStyleVar();
}

void RenderGraphView::Rebuild() {
	const QList<QRhiTextureRenderTarget*>& currentActivatedRenderTargets = mRenderer->getRenderGraphBuilder()->getActivatedRenderTargets();
	if (currentActivatedRenderTargets == mLastActivatedRenderTargets) {
		return;
	}
	mLastActivatedRenderTargets = currentActivatedRenderTargets;

	mCompLeft = nullptr;
	mCompRight = nullptr;
	mNodes.clear();
	mNodeTexutes.clear();
	mLinks.clear();
	mTemplates.clear();

	size_t maxOutSlot = 0;
	QHash<QString, int> renderTargetToNodeIndex;
	QMap<QRhiTexture*, QRhiTextureRenderTarget*> textureToRT;
	for (auto& renderTarget : currentActivatedRenderTargets) {
		QList<QRhiTexture*> rtTextures;
		for (int i = 0; i < renderTarget->description().colorAttachmentCount(); i++) {
			const QRhiColorAttachment* colorAttachment = renderTarget->description().colorAttachmentAt(i);
			QRhiTexture* texture = colorAttachment->resolveTexture() ? colorAttachment->resolveTexture() : colorAttachment->texture();
			if (texture) {
				rtTextures << texture;
				textureToRT[texture] = renderTarget;
			}
		}
		if (renderTarget->description().depthTexture()) {
			rtTextures << renderTarget->description().depthTexture();
			textureToRT[renderTarget->description().depthTexture()] = renderTarget;
		}

		if (rtTextures.isEmpty())
			continue;

		mNodeTexutes << rtTextures;

		GraphEditor::Node node;
		node.mName = renderTarget->name();
		node.mTemplateIndex = rtTextures.size();
		node.mRect = ImRect(0, 0, mOptions.mNodeSlotSize, node.mTemplateIndex * (mOptions.mNodeSlotSize));
		node.mSelected = false;
		maxOutSlot = qMax(maxOutSlot, node.mTemplateIndex);
		renderTargetToNodeIndex[renderTarget->name()] = mNodes.size();
		mNodes << std::move(node);

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
	auto rtPipelinesMap = mRenderer->getRenderGraphBuilder()->getRenderTargetPipelines();
	for (auto rtPipelines : rtPipelinesMap.asKeyValueRange()) {
		for (auto& pipeline : rtPipelines.second) {
			for (int i = 0; i < pipeline->shaderResourceBindings()->bindingCount(); i++) {
				const QRhiShaderResourceBinding* binding = pipeline->shaderResourceBindings()->bindingAt(i);
				QRhiShaderResourceBinding::Data* data = (QRhiShaderResourceBinding::Data*)binding;
				if (data->type == QRhiShaderResourceBinding::SampledTexture) {
					QRhiTexture* refTexture = data->u.stex.texSamplers->tex;
					QRhiTextureRenderTarget* refRt = textureToRT[refTexture];
					if (refRt) {
						GraphEditor::Link link;
						link.mInputNodeIndex = renderTargetToNodeIndex[refRt->name()];
						link.mInputSlotIndex = i;
						link.mOutputNodeIndex = renderTargetToNodeIndex[rtPipelines.first->name()];
						link.mOutputSlotIndex = 0;
						mLinks << std::move(link);
					}
				}
			}
		}
	}

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
		node.mRect = ImRect(xOffset, yOffset, xOffset + mOptions.mNodeSlotSize, yOffset + node.mTemplateIndex * (mOptions.mNodeSlotSize));
	}

	if (bShowFrameComparer) {
		if (mCurrentNodeIndex >= 0 && mCurrentNodeSlotIndex >= 0 && mLastNodeIndex >= 0 && mLastNodeSlotIndex >= 0) {
			mCompLeft = mNodeTexutes[mLastNodeIndex][mLastNodeSlotIndex];
			mCompRight = mNodeTexutes[mCurrentNodeIndex][mCurrentNodeSlotIndex];
		}
		else {
			bShowFrameComparer = false;
		}
	}
}

void RenderGraphView::ShowFrameComparer(float thickness, float leftMinWidth, float rightMinWidth, float splitterLongAxisSize) {
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y));

	ImGui::Begin("FrameComparer", NULL,
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoNav
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_UnsavedDocument);

	auto width = ImGui::GetWindowWidth();
	float left = width * mCompSplitFactor;
	float right = width - left - thickness;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(thickness, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.3f, 1.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImVec4(0.3f, 0.1f, 1.0f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.3f, 0.0f, 1.0f, 1.0f));
	ImGuiID id = window->GetID("##FrameComparer");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (ImVec2(left - 1, 0));
	bb.Max = bb.Min + ImGui::CalcItemSize(ImVec2(thickness + 1, splitterLongAxisSize), 0.0f, 0.0f);
	ImGui::SplitterBehavior(bb, id, ImGuiAxis_X, &left, &right, leftMinWidth, rightMinWidth, 8.0f);
	mCompSplitFactor = left / width;
	ImGui::BeginChild("Left", ImVec2(left, -1), true);
	ImGui::Image(mCompLeft, ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), ImVec2(0.0f, 0.0f), ImVec2(1 * mCompSplitFactor, 1.0f));
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Right", ImVec2(right, -1.0f), true);
	ImGui::Image(mCompRight, ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()), ImVec2(1.0f * mCompSplitFactor, 0.0f), ImVec2(1.0f, 1.0f));
	ImGui::EndChild();
	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(2);
	ImGui::End();

	auto dpr = qApp->devicePixelRatio();
	ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - 60 * dpr, viewport->WorkSize.y - 40 * dpr));
	ImGui::SetNextWindowSize(ImVec2(100 * dpr, 40 * dpr));

	ImGui::Begin("ExitButton", NULL,
		ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoNav
		| ImGuiWindowFlags_NoBackground);
	if (ImGui::Button("Exit")) {
		bShowFrameComparer = false;
		mLastNodeIndex = -1;
		mLastNodeSlotIndex = -1;
		mCompLeft = nullptr;
		mCompRight = nullptr;
	}
	ImGui::End();
}

void RenderGraphView::RequestFitScreen() {
	mFitOnScreen = GraphEditor::Fit_SelectedNodes;
}

QRhiTexture* RenderGraphView::GetCurrentTexture() {
	if (mCurrentNodeIndex >= 0 && mCurrentNodeIndex < mNodeTexutes.size()) {
		return mNodeTexutes[mCurrentNodeIndex].value(mCurrentNodeSlotIndex, nullptr);
	}
	return nullptr;
}

void RenderGraphView::SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected , int slotIndex) {
	if (mCurrentNodeIndex >= 0) {
		mNodes[mCurrentNodeIndex].mSelected = false;
	}
	if (selected) {
		mNodes[nodeIndex].mSelected = selected;
		if (ImGui::GetIO().KeyCtrl && mCurrentNodeIndex >= 0 && mCurrentNodeSlotIndex >= 0  && nodeIndex >=0 && slotIndex>=0) {
			bShowFrameComparer = true;
			mCompSplitFactor = 0.5f;
			mLastNodeIndex = mCurrentNodeIndex;
			mLastNodeSlotIndex = mCurrentNodeSlotIndex;
			mCompLeft = mNodeTexutes[mCurrentNodeIndex][mCurrentNodeSlotIndex];
			mCompRight = mNodeTexutes[nodeIndex][slotIndex];
		}
		mCurrentNodeIndex = nodeIndex;
		mCurrentNodeSlotIndex = slotIndex;
	}
}

void RenderGraphView::CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) {
	const float textureNameFontSize = 12.0f * mViewState.mFactor;
	for (int i = 0; i < mNodeTexutes[nodeIndex].size(); i++) {
		ImVec2 min(rectangle.Min.x, rectangle.Min.y + (mOptions.mNodeSlotSize) * mViewState.mFactor * i);
		ImVec2 max(min.x + mOptions.mNodeSlotSize * mViewState.mFactor, min.y + mOptions.mNodeSlotSize * mViewState.mFactor);
		if (mNodeTexutes[nodeIndex][i]->arraySize() == 0 && !mNodeTexutes[nodeIndex][i]->flags().testFlag(QRhiTexture::CubeMap)) {
			drawList->AddImage((ImTextureID)mNodeTexutes[nodeIndex][i], min, max);
		}
		ImVec2 textSize = ImGui::CalcTextSize(mNodeTexutes[nodeIndex][i]->name().data());
		float offset = (rectangle.GetWidth() - textSize.x * textureNameFontSize / textSize.y) / 2;
		drawList->AddText(NULL, textureNameFontSize , ImVec2(min.x + offset, min.y), ImGui::GetColorU32(ImGuiCol_Text), mNodeTexutes[nodeIndex][i]->name().data());
		if (nodeIndex == mCurrentNodeIndex && i == mCurrentNodeSlotIndex) {
			drawList->AddRect(min, max, IM_COL32(79, 110, 242, 255), 0, 0, 3);
		}
		else {
			drawList->AddRect(min, max, IM_COL32(200, 200, 200, 255), 0, 0, 1);
		}
	}
}

const GraphEditor::Template RenderGraphView::GetTemplate(GraphEditor::TemplateIndex index) {
	return mTemplates[index];
}

const size_t RenderGraphView::GetNodeCount() {
	return mNodes.size();
}

const GraphEditor::Node RenderGraphView::GetNode(GraphEditor::NodeIndex index) {
	return mNodes[index];
}

const GraphEditor::Link RenderGraphView::GetLink(GraphEditor::LinkIndex index) {
	return mLinks[index];
}

#endif