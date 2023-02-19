#ifndef DebugUiPainter_h__
#define DebugUiPainter_h__

#include "Render/Painter/ImGuiPainter.h"
#include "Render/ISceneRenderComponent.h"
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>
#include <ImGuizmo.h>
#include "Utils/DebugUtils.h"
#include "Render/IRenderPass.h"
#include "Render/Painter/TexturePainter.h"
#include "DetailView/QPropertyHandle.h"
#include "Render/RHI/QRhiGraphicsPipelineBuilder.h"
#include "GraphEditor.h"
#include "Render/Renderer/QWindowRenderer.h"
#include "FrameGraphView.h"

class QDebugUIPainter :public ImGuiPainter {
protected:
	QWindowRenderer* mRenderer;
	ImGuiWindowFlags mViewportBarFlags;
	ImGuizmo::OPERATION mOperation = ImGuizmo::OPERATION::TRANSLATE;
	bool bShowFrameGraph;
	bool bLineMode;
	GraphEditor::FitOnScreen mFitOnScreen = GraphEditor::Fit_AllNodes;
	FrameGraphDelegate mFrameGraphDelegate;
	GraphEditor::Options mFrameGraphOption;
	GraphEditor::ViewState mFrameGraphViewState;
public:
	void setupDebugIdTexture(QRhiTexture* texture);
	QDebugUIPainter(QWindowRenderer* inRenderer);
	void resourceUpdate(QRhiResourceUpdateBatch* batch) override;
	void compile() override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override {
		if (mDebugIdTexture) {
			cmdBuffer->setGraphicsPipeline(mOutlinePipeline.get());
			cmdBuffer->setViewport(QRhiViewport(0, 0, renderTarget->pixelSize().width(), renderTarget->pixelSize().height()));
			cmdBuffer->setShaderResources(mOutlineBindings.get());
			cmdBuffer->draw(4);
		}
		ImGuiPainter::paint(cmdBuffer, renderTarget);
	}
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
private:
	QRhiTexture* mDebugIdTexture = nullptr;
	QRhiReadbackResult mReadReult;
	QRhiReadbackDescription mReadDesc;
	QPoint mReadPoint;
	QScopedPointer<QRhiGraphicsPipeline> mOutlinePipeline;
	QScopedPointer<QRhiSampler> mOutlineSampler;
	QScopedPointer<QRhiShaderResourceBindings> mOutlineBindings;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
};

#endif // DebugUiPainter_h__
