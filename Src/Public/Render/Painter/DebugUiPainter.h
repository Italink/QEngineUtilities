#ifdef QENGINE_WITH_EDITOR

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

class FrameGraphView;

class QDebugUIPainter :public ImGuiPainter {
protected:
	QWindowRenderer* mRenderer;
	ImGuiWindowFlags mViewportBarFlags;
	ImGuizmo::OPERATION mOperation = ImGuizmo::OPERATION::TRANSLATE;
	bool bUseLineMode = false;
	bool bShowFrameGraph = false;
	bool bShowStats = false;
	bool bDrawOuterline = true;
	QSharedPointer<FrameGraphView> mFrameGraphView;
public:
	QDebugUIPainter(QWindowRenderer* inRenderer);
	void setupDebugIdTexture(QRhiTexture* texture);
	void resourceUpdate(QRhiResourceUpdateBatch* batch) override;
	void compile() override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
private:
	QRhiTexture* mDebugIdTexture = nullptr;
	QRhiTexture* mOutputTexture = nullptr;
	QRhiReadbackResult mReadReult;
	QRhiReadbackDescription mReadDesc;
	QPoint mReadPoint;
	QScopedPointer<QRhiGraphicsPipeline> mOutlinePipeline;
	QScopedPointer<QRhiSampler> mOutlineSampler;
	QScopedPointer<QRhiShaderResourceBindings> mOutlineBindings;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	ImVec4 mActiveColor = ImColor(100, 150, 255);
};

#endif // DebugUiPainter_h__

#endif // QENGINE_WITH_EDITOR