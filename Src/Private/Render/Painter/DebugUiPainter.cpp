#ifdef QENGINE_WITH_EDITOR

#include "Render/Painter/DebugUiPainter.h"
#include "FrameGraphView.h"
#include "Render/RHI/QRhiWindow.h"
#include "Utils/ImGuiWidgets.h"

void QDebugUIPainter::setupDebugIdTexture(QRhiTexture* texture) {
	mDebugIdTexture = texture;
}

QDebugUIPainter::QDebugUIPainter(QWindowRenderer* inRenderer) 
	: mRenderer(inRenderer)
	, mFrameGraphView(new FrameGraphView())
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

	setupPaintFunctor([this]() {
		QCamera* camera = mRenderer->getCamera();
		if (camera) {
			auto& io = ImGui::GetIO();

			QMatrix4x4 View = camera->getMatrixView();
			QMatrix4x4 Clip = camera->getMatrixClip();
			ISceneRenderComponent* currComponent = qobject_cast<ISceneRenderComponent*>(mRenderer->getCurrentObject());
			if (currComponent) {
				QMatrix4x4 MAT;
				QMatrix4x4 Model = currComponent->calculateMatrixModel();

				ImGuizmo::BeginFrame();
				ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
				ImGuizmo::Manipulate(View.constData(), Clip.constData(), mOperation, ImGuizmo::LOCAL, Model.data(), NULL, NULL, NULL, NULL);
				currComponent->setTransform(Model);
				if (QPropertyHandle* position = QPropertyHandle::Find(currComponent, "Transform.Position"))
					position->RefreshBinder();
				if (QPropertyHandle* rotation = QPropertyHandle::Find(currComponent, "Transform.Rotation"))
					rotation->RefreshBinder();
				if (QPropertyHandle* scale = QPropertyHandle::Find(currComponent, "Transform.Scale"))
					scale->RefreshBinder();
			}
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
			ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 60));
			ImGui::Begin("ViewportBar", 0, mViewportBarFlags);
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
			if (ImGui::IconButton(getImageId("polygon"), ImVec2(30, 30), "Polygon", bUseLineMode ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				bUseLineMode = !bUseLineMode;
			}
			ImGui::SameLine();
			QString speed =  QString::number(camera->getMoveSpeed(), 'f', 2);
			if (ImGui::IconButton(getImageId("camera"), ImVec2(30, 30), speed.toLocal8Bit().data(), ImVec4(1, 1, 1, 1), 5)) {
				ImGui::OpenPopup("Move Speed Slider");
				ImGui::SetNextWindowPos(ImVec2(155,60));
			}
			if (ImGui::BeginPopup("Move Speed Slider")) {
				ImGui::VSliderFloat("##", ImVec2(40, 100), &camera->getMoveSpeedRef(), 0.01, 5);
				ImGui::EndPopup();
			}
			ImGui::RenderFrame(ImVec2(viewport->WorkSize.x - 465, viewport->WorkPos.y + 8), ImVec2(viewport->WorkSize.x - 280, viewport->WorkPos.y + 48), ImGui::GetColorU32(ImGuiCol_Button), true, GImGui->Style.FrameRounding);
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
			ImGui::SameLine(viewport->WorkSize.x - 460); 
			if (ImGui::IconButton(getImageId("select"), ImVec2(30, 30), "", currComponent == nullptr ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mRenderer->setCurrentObject(nullptr);
			}
			ImGui::SameLine(viewport->WorkSize.x - 415); 
			if (ImGui::IconButton(getImageId("translate"), ImVec2(30, 30), "", currComponent != nullptr && mOperation == ImGuizmo::OPERATION::TRANSLATE ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mOperation = ImGuizmo::OPERATION::TRANSLATE;
			}
			ImGui::SameLine(viewport->WorkSize.x - 370); 
			if (ImGui::IconButton(getImageId("rotate"), ImVec2(30, 30), "", currComponent != nullptr && mOperation == ImGuizmo::OPERATION::ROTATE ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mOperation = ImGuizmo::OPERATION::ROTATE;
			}
			ImGui::SameLine(viewport->WorkSize.x - 325); 
			if (ImGui::IconButton(getImageId("scale"), ImVec2(30, 30), "", currComponent != nullptr && mOperation == ImGuizmo::OPERATION::SCALE ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				mOperation = ImGuizmo::OPERATION::SCALE;
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();
			ImGui::SameLine(viewport->WorkSize.x - 270);
			if (ImGui::IconButton(getImageId("graph"), ImVec2(30, 30), "FrameGraph", bShowFrameGraph ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				bShowFrameGraph = !bShowFrameGraph;
			}

			ImGui::SameLine(viewport->WorkSize.x - 100);
			if (ImGui::IconButton(getImageId("stats"), ImVec2(30, 30), "Stats", bShowStats ? mActiveColor : ImVec4(1, 1, 1, 1), 5)) {
				bShowStats = !bShowStats;
			}
			ImGui::PopStyleVar();
			ImGui::End();
			if (bUseLineMode)
				QRhiGraphicsPipelineBuilder::setPolygonModeOverride(QRhiGraphicsPipeline::Line);
			else
				QRhiGraphicsPipelineBuilder::clearPolygonModeOverride();
			if (bShowFrameGraph) {
				ImGui::SetNextWindowPos(ImVec2(0, 50));
				ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - 50));
				ImGui::Begin("Frame Graph", NULL, mViewportBarFlags);
				mFrameGraphView->Show();
				ImGui::End();
				mOutputTexture = mFrameGraphView->GetCurrentTexture();
				mRenderer->TryOverrideOutputTexture(mOutputTexture);
			}
			if (bShowStats) {
				ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - 250, viewport->WorkSize.y - 100));
				ImGui::SetNextWindowSize(ImVec2(200, 200));
				ImGui::Begin("Stats", 0, mViewportBarFlags);
				ImGui::TextColored(ImColor(0, 255, 0), "FPS          \t%d", mRenderer->getWindow()->getFps());
				ImGui::TextColored(ImColor(0, 255, 0), "CPU Time\t%.2f ms", mRenderer->getWindow()->getCpuFrameTime());
				ImGui::TextColored(ImColor(0, 255, 0), "GPU Time\t%.2f ms", mRenderer->getWindow()->getGpuFrameTime());
				ImGui::End();
			}

		}
	});
	setupRhi(mRenderer->getRhi());
	setupWindow(mRenderer->getWindow());
	setupSampleCount(mRenderer->sampleCount());
	setupRenderPassDesc(mRenderer->renderPassDescriptor());
}

void QDebugUIPainter::resourceUpdate(QRhiResourceUpdateBatch* batch) {
	ImGuiPainter::resourceUpdate(batch);
	if (mDebugIdTexture) {
		if (!mReadPoint.isNull() && !ImGui::GetIO().MouseDownOwned[ImGuiMouseButton_Left]) {
			mReadDesc.setTexture(mDebugIdTexture);
			mReadReult.completed = [this]() {
				if (!mReadPoint.isNull()) {
					const uchar* p = reinterpret_cast<const uchar*>(mReadReult.data.constData());
					int offset = (mReadReult.pixelSize.width() * mReadPoint.y() + mReadPoint.x()) * 4;
					uint32_t Id = p[offset] + p[offset + 1] * 256 + p[offset + 2] * 256 * 256 + p[offset + 3] * 256 * 256 * 256;
					mRenderer->setCurrentObject(mRenderer->getComponentById(Id));
					mReadPoint = { 0,0 };
				}
			};
			batch->readBackTexture(mReadDesc, &mReadReult);
			mRhi->finish();
		}
		ISceneRenderComponent* CurrComponent = qobject_cast<ISceneRenderComponent*>(mRenderer->getCurrentObject());
		QVector4D ID(-1, -1, -1, -1);
		if (CurrComponent) {
			ID = DebugUtils::convertIdToVec4(CurrComponent->getID());
		}
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(QVector4D), &ID);
	}
}

void QDebugUIPainter::compile() {
	mFrameGraphView->Rebuild(mRenderer->getFrameGarph());
	registerImage("select", QImage(":/Resources/mouse-arrow.png"));
	registerImage("translate", QImage(":/Resources/translate.png"));
	registerImage("rotate", QImage(":/Resources/rotate.png"));
	registerImage("scale", QImage(":/Resources/scale.png"));
	registerImage("stats", QImage(":/Resources/stats.png"));
	registerImage("polygon", QImage(":/Resources/polygon.png"));
	registerImage("camera", QImage(":/Resources/camera.png"));
	registerImage("graph", QImage(":/Resources/graph.png"));
	ImGuiPainter::compile();
	if (mDebugIdTexture == nullptr)
		return;
	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D)));
	mUniformBuffer->create();

	mOutlineSampler.reset(mRhi->newSampler(QRhiSampler::Nearest,
		QRhiSampler::Nearest,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	mOutlineSampler->create();
	mOutlinePipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	mOutlinePipeline->setTargetBlends({ blendState });
	mOutlinePipeline->setSampleCount(mSampleCount);

	QString vsCode = R"(#version 450
	layout (location = 0 ) out vec2 vUV;
	out gl_PerVertex{
		vec4 gl_Position;
	};
	void main() {
		vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
		gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
		%1
	}
	)";
	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());

	QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
	layout (location = 0) in vec2 vUV;
	layout (location = 0) out vec4 outFragColor;

	layout (binding = 0) uniform sampler2D uDebugId;
	layout (binding = 1) uniform DebugId{
		vec4 ID;
	}Current;

	void main() {
		vec2 texOffset = 1.0 / textureSize(uDebugId, 0);		// gets size of single texel
		
		int count = 0;

		count += (texture(uDebugId,vUV) == Current.ID ? 1 : 0 );		
		count += (texture(uDebugId,vUV+vec2(texOffset.x,0)) == Current.ID ? 1 : 0 );
		count += (texture(uDebugId,vUV-vec2(texOffset.x,0)) == Current.ID ? 1 : 0 );
		count += (texture(uDebugId,vUV+vec2(0,texOffset.y)) == Current.ID ? 1 : 0 );
		count += (texture(uDebugId,vUV-vec2(0,texOffset.y)) == Current.ID ? 1 : 0 );

		if(count>0&&count<5){
			outFragColor = vec4(1.0,0.8,0.4,1.0);
		}
		else{
			outFragColor = vec4(0);
		}
	}
	)");
	mOutlinePipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
		});
	QRhiVertexInputLayout inputLayout;

	mOutlineBindings.reset(mRhi->newShaderResourceBindings());

	mOutlineBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mDebugIdTexture,mOutlineSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
		});

	mOutlineBindings->create();
	mOutlinePipeline->setVertexInputLayout(inputLayout);
	mOutlinePipeline->setShaderResourceBindings(mOutlineBindings.get());
	mOutlinePipeline->setRenderPassDescriptor(mRenderPassDesc);
	mOutlinePipeline->create();
}

void QDebugUIPainter::paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) {
	if (mDebugIdTexture && bDrawOuterline && !mOutputTexture) {
		cmdBuffer->setGraphicsPipeline(mOutlinePipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, renderTarget->pixelSize().width(), renderTarget->pixelSize().height()));
		cmdBuffer->setShaderResources(mOutlineBindings.get());
		cmdBuffer->draw(4);
	}
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
			QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
			if (pressedPos == QCursor::pos() && mouse->button() == Qt::LeftButton) {
				QPoint pt = mouse->pos() * qApp->devicePixelRatio();
				if (mRhi->isYUpInNDC()) {
					pt.setY(mWindow->height() - pt.y());
				}
				mReadPoint = pt;
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
				mFrameGraphView->RequestFitScreen();
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

#endif // QENGINE_WITH_EDITOR