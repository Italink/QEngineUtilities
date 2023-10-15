#ifdef QENGINE_WITH_EDITOR

#ifndef DebugUiPainter_h__
#define DebugUiPainter_h__

#include "GraphEditor.h"
#include "QEngineLaunchAPI.h"
#include "Render/ISceneRenderComponent.h"
#include "Render/RenderGraph/Painter/ImGuiPainter.h"
#include "Render/QPrimitiveRenderProxy.h"
#include "Utils/DebugUtils.h"
#include <ImGuizmo.h>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>

class RenderGraphView;

class QENGINELAUNCH_API QDebugUIPainter: public ImGuiPainter {
	Q_OBJECT
protected:
	ImGuiWindowFlags mViewportBarFlags;
	ImGuizmo::OPERATION mOperation = ImGuizmo::OPERATION::TRANSLATE;
	bool bUseLineMode = false;
	bool bShowFrameGraph = false;
	bool bShowStats = false;
	bool bDrawOuterline = true;
	QSharedPointer<RenderGraphView> mRenderGraphView;
public:
	QDebugUIPainter(IRenderer* inRenderer);

	void setup(QRenderGraphBuilder& builder, QRhiRenderTarget* rt) override;
	void resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi) override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
Q_SIGNALS:
	void mouseClicked(QPoint point);
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
	void refreshEditor(QRhiCamera* camera, ISceneRenderComponent* comp, QMatrix4x4 compModelMatrix);
private:
	QRhi* mRhi = nullptr;
	IRenderer* mRenderer = nullptr;
	ImVec4 mActiveColor = ImColor(100, 150, 255);
};

#endif // DebugUiPainter_h__

#endif // QENGINE_WITH_EDITOR