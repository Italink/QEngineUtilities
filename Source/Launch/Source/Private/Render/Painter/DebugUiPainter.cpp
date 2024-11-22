#ifdef QENGINE_WITH_EDITOR

#include "Render/Painter/DebugUiPainter.h"
#include "RenderGraphView.h"
#include "Render/RHI/QRhiWindow.h"
#include "Utils/ImGuiWidgets.h"
#include "QEngineEditorStyleManager.h"
#include <QFile>
#include <QTime>
#include "Render/IRenderer.h"
#include "Utils/QRhiCamera.h"
#include "DetailView/QPropertyHandle.h"

QDebugUIPainter::QDebugUIPainter(IRenderer* inRenderer)
	: mRenderer(inRenderer)
	, mRhi(inRenderer->rhi())
	, mRenderGraphView(new RenderGraphView(inRenderer))
{
	mViewportBarFlags = ImGuiWindowFlags_NoTitleBar;
	mViewportBarFlags |= ImGuiWindowFlags_NoScrollbar;
	mViewportBarFlags |= ImGuiWindowFlags_NoMove;
	mViewportBarFlags |= ImGuiWindowFlags_NoResize;
	mViewportBarFlags |= ImGuiWindowFlags_NoCollapse;
	mViewportBarFlags |= ImGuiWindowFlags_NoNav;
	mViewportBarFlags |= ImGuiWindowFlags_NoBackground;
	mViewportBarFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	mViewportBarFlags |= ImGuiWindowFlags_UnsavedDocument;

	ImGui::SetCurrentContext(mImGuiContext);
	ImGuiIO& io = ImGui::GetIO();
	QFile file(QEngineEditorStyleManager::Instance()->getFontFilePath());
	if (file.open(QIODevice::ReadOnly)) {
		QByteArray fontData = file.readAll();
		ImFontConfig config;
		config.OversampleH = 8;
		config.OversampleV = 8;
		io.Fonts->AddFontFromMemoryTTF(fontData.data(), fontData.size(), 16, &config);
		io.Fonts->Build();
	}
	const QSize iconSize(30, 30);

	registerImage("select", QImage(":/Resources/mouse-arrow.png").scaled(iconSize,Qt::IgnoreAspectRatio,Qt::TransformationMode::SmoothTransformation));
	registerImage("translate", QImage(":/Resources/translate.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("rotate", QImage(":/Resources/rotate.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("scale", QImage(":/Resources/scale.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("stats", QImage(":/Resources/stats.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("polygon", QImage(":/Resources/polygon.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("camera", QImage(":/Resources/camera.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("graph", QImage(":/Resources/graph.png").scaled(iconSize, Qt::IgnoreAspectRatio, Qt::TransformationMode::SmoothTransformation));
	registerImage("tips", QImage(":/Resources/tips.png"));

	setupWindow(mRenderer->maybeWindow());
	setupPaintFunctor([this](ImGuiContext* Ctx) {
		ImGui::SetCurrentContext(Ctx);
		QRhiCamera* camera = mRenderer->getCamera();
		if (camera) {
			auto& io = ImGui::GetIO();
			QMatrix4x4 View = camera->getViewMatrix();
			QMatrix4x4 Clip = camera->getProjectionMatrix();
			ISceneRenderComponent* currComponent = qobject_cast<ISceneRenderComponent*>(mRenderer->getCurrentObject());
			QMatrix4x4 Model;
			if (currComponent) {
				Model = currComponent->getModelMatrix();
				if (!bShowFrameGraph) {
					ImGuizmo::BeginFrame();
					ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
					ImGuizmo::Manipulate(View.constData(), Clip.constData(), mOperation, ImGuizmo::LOCAL, Model.data(), NULL, NULL, NULL, NULL);
				}
			}
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			auto dpr = qApp->devicePixelRatio();
			ImVec2 ButtonSize = ImVec2(20 * dpr, 20 * dpr);
			ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
			ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 60 * dpr));
			ImGui::Begin("ViewportBar", 0, mViewportBarFlags);
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
			if (ImGui::IconButton(getImageId("polygon"), ButtonSize, "Polygon", bUseLineMode ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				bUseLineMode = !bUseLineMode;
			}
			ImGui::SameLine();
			QString speed =  QString::number(camera->getMoveSpeed(), 'f', 2);
			if (ImGui::IconButton(getImageId("camera"), ButtonSize, speed.toLocal8Bit().data(), ImVec4(1, 1, 1, 1), 5)) {
				ImGui::OpenPopup("Move Speed Slider");
				ImGui::SetNextWindowPos(ImVec2(120 * dpr, 40 * dpr));
			}
			auto it = ImGui::GetWindowSize();
			if (ImGui::BeginPopup("Move Speed Slider")) {
				ImGui::VSliderFloat("##", ImVec2(40 * dpr, 100 * dpr), &camera->getMoveSpeedRef(), 0.01, 2 , "%.2f");
				ImGui::EndPopup();
			}
			auto ij = ImGui::GetWindowSize();
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
			int right = 350 * dpr;
			int offset = 30 * dpr;
			ImGui::SameLine(viewport->WorkSize.x - right);
			if (ImGui::IconButton(getImageId("select"), ButtonSize, "", currComponent == nullptr ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mRenderer->setCurrentObject(nullptr);
			}
			right -= offset;
			ImGui::SameLine(viewport->WorkSize.x - right);
			if (ImGui::IconButton(getImageId("translate"), ButtonSize, "", currComponent != nullptr && mOperation == ImGuizmo::OPERATION::TRANSLATE ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			right -= offset;
			ImGui::SameLine(viewport->WorkSize.x - right);
			if (ImGui::IconButton(getImageId("rotate"), ButtonSize, "", currComponent != nullptr && mOperation == ImGuizmo::OPERATION::ROTATE ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mOperation = ImGuizmo::OPERATION::ROTATE;
			}
			right -= offset;
			ImGui::SameLine(viewport->WorkSize.x - right);
			if (ImGui::IconButton(getImageId("scale"), ButtonSize, "", currComponent != nullptr && mOperation == ImGuizmo::OPERATION::SCALE ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mOperation = ImGuizmo::OPERATION::SCALE;
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
			right -= offset;
			right -= 5 * dpr;
			ImGui::SameLine(viewport->WorkSize.x - right);
			if (ImGui::IconButton(getImageId("graph"), ButtonSize, "FrameGraph", bShowFrameGraph ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				bShowFrameGraph = !bShowFrameGraph;
			}
			right -= 150 * dpr;
			ImGui::SameLine(viewport->WorkSize.x - right);
			if (ImGui::IconButton(getImageId("stats"), ButtonSize, "Stats", bShowStats ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				bShowStats = !bShowStats;
			}
			ImGui::PopStyleVar();
			ImGui::End();
			
			static bool showTips = true;
			static int FrameCounter = 500;
			if (showTips || FrameCounter) {
				if(ImGui::GetIO().MouseDown[ImGuiMouseButton_Left] 
					|| ImGui::GetIO().MouseDown[ImGuiMouseButton_Right]
					|| ImGui::GetIO().MouseDown[ImGuiMouseButton_Middle]) {
					showTips = false;
				}

				ImGui::SetNextWindowBgAlpha(0);
				ImVec2 Size(700, 170);
				ImVec2 Pos(viewport->WorkSize.x - Size.x, viewport->WorkSize.y - Size.y);
				ImGui::SetNextWindowPos(Pos);
				ImGui::SetNextWindowSize(Size);
				ImGui::Begin("Tips", NULL,
					ImGuiWindowFlags_NoTitleBar
					| ImGuiWindowFlags_NoScrollbar
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoNav
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoBringToFrontOnFocus
					| ImGuiWindowFlags_UnsavedDocument);
				ImGui::Image(getImageId("tips"), Size,ImVec2(0,0),ImVec2(1,1),ImVec4(1,1,1,FrameCounter/500.0f));
				ImGui::End();
				if (!showTips) {
					FrameCounter--;
				}
			}

			//if (bUseLineMode)
			//	QPrimitiveRenderProxy::setPolygonModeOverride(QRhiGraphicsPipeline::Line);
			//else
			//	QPrimitiveRenderProxy::clearPolygonModeOverride();
			if (bShowFrameGraph) {
				mRenderGraphView->Show();
				//mOutputTexture = mRenderGraphView->GetCurrentTexture();
				//mRenderer->setOverrideOutput(mOutputTexture);
			}
			if (bShowStats) {
				if (QRhiWindow* Window = qobject_cast<QRhiWindow*>(mRenderer->maybeWindow())) {
					ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - 180 * dpr, viewport->WorkSize.y - 100 * dpr));
					ImGui::SetNextWindowSize(ImVec2(200, 200));
					ImGui::Begin("Stats", 0, mViewportBarFlags);
					ImGui::TextColored(ImColor(0, 255, 0), "FPS          \t%d", Window->getFps());
					ImGui::TextColored(ImColor(0, 255, 0), "CPU Time\t%.2f ms", Window->getCpuFrameTime());
					ImGui::TextColored(ImColor(0, 255, 0), "GPU Time\t%.2f ms", Window->getGpuFrameTime());
					ImGui::End();
				}
			}
			QMetaObject::invokeMethod(this, std::bind(&QDebugUIPainter::refreshEditor, this, camera, currComponent, Model), Qt::ConnectionType::QueuedConnection);
		}
	});
}

void QDebugUIPainter::setup(QRenderGraphBuilder& builder, QRhiRenderTarget* rt)
{
	ImGuiPainter::setup(builder, rt);
}

void QDebugUIPainter::resourceUpdate(QRhiResourceUpdateBatch* batch, QRhi* rhi)
{
	ImGuiPainter::resourceUpdate(batch, rhi);
}

void QDebugUIPainter::paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) {
	ImGuiPainter::paint(cmdBuffer, renderTarget);
}

bool QDebugUIPainter::eventFilter(QObject* watched, QEvent* event) {
	static QPoint pressedPos;
	if (watched != nullptr) {
		switch (event->type()) {
		case QEvent::MouseButtonPress:
			pressedPos = QCursor::pos();
			break;
		case QEvent::MouseButtonRelease: {
			if (!bShowFrameGraph) {
				QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
				if (mouse->button() == Qt::LeftButton) {
					QPoint pt = mouse->pos() * qApp->devicePixelRatio();
					if (mRhi->isYUpInNDC()) {
						pt.setY(getWindow()->height() - pt.y());
					}
					Q_EMIT mouseClicked(pt);
				}
			}
			pressedPos = { 0,0 };
			break;
		}
		case QEvent::MouseMove: {
			if (qApp->mouseButtons() & Qt::LeftButton) {
			}
			break;
		}
		case QEvent::KeyPress: {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_W) {
				mOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			else if (keyEvent->key() == Qt::Key_E) {
				mOperation = ImGuizmo::OPERATION::ROTATE;
			}
			else if (keyEvent->key() == Qt::Key_R) {
				mOperation = ImGuizmo::OPERATION::SCALE;
			}
			else if (keyEvent->key() == Qt::Key_F) {
				mRenderGraphView->RequestFitScreen();
			}
			break;
		}
		case QEvent::KeyRelease: {
			break;
		}
		}
	}
	return ImGuiPainter::eventFilter(watched, event);
}

void QDebugUIPainter::refreshEditor(QRhiCamera* camera, ISceneRenderComponent* comp,  QMatrix4x4 compModelMatrix)
{
	if (camera) {
		if (QPropertyHandle* position = QPropertyHandle::Find(camera, "Position"))
			position->refreshBinder();
		if (QPropertyHandle* rotation = QPropertyHandle::Find(camera, "Rotation"))
			rotation->refreshBinder();
		if (QPropertyHandle* moveSpeed = QPropertyHandle::Find(camera, "MoveSpeed"))
			moveSpeed->refreshBinder();
	}
	if (comp) {
		if(QPropertyHandle* transform = QPropertyHandle::FindOrCreate(comp, "Transform"))
			transform->setValue(compModelMatrix, "Move");
		if (QPropertyHandle* position = QPropertyHandle::Find(comp, "Transform.Position"))
			position->refreshBinder();
		if (QPropertyHandle* rotation = QPropertyHandle::Find(comp, "Transform.Rotation"))
			rotation->refreshBinder();
		if (QPropertyHandle* scale = QPropertyHandle::Find(comp, "Transform.Scale"))
			scale->refreshBinder();
	}
}

#endif // QENGINE_WITH_EDITOR