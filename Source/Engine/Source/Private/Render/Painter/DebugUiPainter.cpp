#ifdef QENGINE_WITH_EDITOR

#include "Render/Painter/DebugUiPainter.h"
#include "FrameGraphView.h"
#include "Render/RHI/QRhiWindow.h"
#include "Utils/ImGuiWidgets.h"
#include "QEngineEditorStyleManager.h"
#include "QFile"

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

	ImGui::SetCurrentContext(mImGuiContext);
	ImGuiIO& io = ImGui::GetIO();
	QFile file(QEngineEditorStyleManager::Instance()->GetFontFilePath());
	if (file.open(QIODevice::ReadOnly)) {
		QByteArray fontData = file.readAll();
		ImFontConfig config;
		config.OversampleH = 5;
		config.OversampleV = 5;
		io.Fonts->AddFontFromMemoryTTF(fontData.data(), fontData.size(), 23, &config);
		io.Fonts->Build();
	}

	setupPaintFunctor([this](ImGuiContext* Ctx) {
		ImGui::SetCurrentContext(Ctx);
		QCamera* camera = mRenderer->getCamera();
		if (camera) {
			auto& io = ImGui::GetIO();
			QMatrix4x4 View = camera->getViewMatrix();
			QMatrix4x4 Clip = camera->getProjectionMatrix();
			ISceneRenderComponent* currComponent = qobject_cast<ISceneRenderComponent*>(mRenderer->getCurrentObject());
			if (currComponent && !bShowFrameGraph) {
				QMatrix4x4 MAT;
				QMatrix4x4 Model = currComponent->getModelMatrix();
				ImGuizmo::BeginFrame();
				ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
				ImGuizmo::Manipulate(View.constData(), Clip.constData(), mOperation, ImGuizmo::LOCAL, Model.data(), NULL, NULL, NULL, NULL);
				auto handler = QPropertyHandle::Find(currComponent, "Transform");
				handler->SetValue(Model, "Move");
				if (QPropertyHandle* position = QPropertyHandle::Find(currComponent, "Transform.Position"))
					position->RefreshBinder();
				if (QPropertyHandle* rotation = QPropertyHandle::Find(currComponent, "Transform.Rotation"))
					rotation->RefreshBinder();
				if (QPropertyHandle* scale = QPropertyHandle::Find(currComponent, "Transform.Scale"))
					scale->RefreshBinder();
			}
			if (QPropertyHandle* position = QPropertyHandle::Find(camera, "Position"))
				position->RefreshBinder();
			if (QPropertyHandle* rotation = QPropertyHandle::Find(camera, "Rotation"))
				rotation->RefreshBinder();
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
			if (bUseLineMode)
				QRhiGraphicsPipelineBuilder::setPolygonModeOverride(QRhiGraphicsPipeline::Line);
			else
				QRhiGraphicsPipelineBuilder::clearPolygonModeOverride();
			if (bShowFrameGraph) {
				mFrameGraphView->Show();
				mOutputTexture = mFrameGraphView->GetCurrentTexture();
				mRenderer->setOverrideOutput(mOutputTexture);
			}
			if (bShowStats) {
				ImGui::SetNextWindowPos(ImVec2(viewport->WorkSize.x - 180 * dpr, viewport->WorkSize.y - 100*dpr));
				ImGui::SetNextWindowSize(ImVec2(200, 200));
				ImGui::Begin("Stats", 0, mViewportBarFlags);
				ImGui::TextColored(ImColor(0, 255, 0), "FPS          \t%d", mRenderer->getRhiWindow()->getFps());
				ImGui::TextColored(ImColor(0, 255, 0), "CPU Time\t%.2f ms", mRenderer->getRhiWindow()->getCpuFrameTime());
				ImGui::TextColored(ImColor(0, 255, 0), "GPU Time\t%.2f ms", mRenderer->getRhiWindow()->getGpuFrameTime());
				ImGui::End();
			}

		}
	});
	setupRhi(mRenderer->getRhi());
	setupWindow(mRenderer->getRhiWindow());
	setupSampleCount(mRenderer->sampleCount());
	setupRenderPassDesc(mRenderer->renderPassDescriptor());
}

void QDebugUIPainter::resourceUpdate(QRhiResourceUpdateBatch* batch) {
	ImGuiPainter::resourceUpdate(batch);
	if (mDebugIdTexture) {
		if (!mReadPoint.isNull() && !ImGui::GetIO().MouseDownOwned[ImGuiMouseButton_Left]) {
			mReadDesc.setTexture(mDebugIdTexture);
			mReadReult.completed = [Pos = mReadPoint,this]() {
				if (!Pos.isNull()) {
					const uchar* p = reinterpret_cast<const uchar*>(mReadReult.data.constData());
					int offset = (mReadReult.pixelSize.width() * Pos.y() + Pos.x()) * 4;
					uint32_t Id = p[offset] + p[offset + 1] * 256 + p[offset + 2] * 256 * 256 + p[offset + 3] * 256 * 256 * 256;
					mRenderer->setCurrentObject(mRenderer->getComponentById(Id));
				}
			};
			batch->readBackTexture(mReadDesc, &mReadReult);
			mRhi->finish();
			mReadPoint = { 0,0 };
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