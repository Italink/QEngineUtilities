#include "DebugUiPainter.h"

void QDebugUIPainter::setupDebugIdTexture(QRhiTexture* texture) {
	mDebugIdTexture = texture;
}

QDebugUIPainter::QDebugUIPainter(QWindowRenderer* inRenderer) : mRenderer(inRenderer) {
	mViewportBarFlags = ImGuiWindowFlags_NoTitleBar;
	mViewportBarFlags |= ImGuiWindowFlags_NoScrollbar;
	mViewportBarFlags |= ImGuiWindowFlags_NoMove;
	mViewportBarFlags |= ImGuiWindowFlags_NoResize;
	mViewportBarFlags |= ImGuiWindowFlags_NoCollapse;
	mViewportBarFlags |= ImGuiWindowFlags_NoNav;
	mViewportBarFlags |= ImGuiWindowFlags_NoBackground;
	mViewportBarFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	mViewportBarFlags |= ImGuiWindowFlags_UnsavedDocument;

	mFrameGraphOption.mRenderGrid = true;
	mFrameGraphOption.mDisplayLinksAsCurves = true;
	mFrameGraphOption.mBackgroundColor = ImColor(0, 0, 0, 0);
	mFrameGraphOption.mGridColor = ImColor(128, 128, 128, 128);
	mFrameGraphOption.mGridColor2 = ImColor(128, 128, 128, 64);
	//mFrameGraphOption.mQuadSelection = ImColor(0, 0, 0, 0);
	//mFrameGraphOption.mQuadSelectionBorder = ImColor(0, 0, 0, 0);
	//mFrameGraphOption.mFrameFocus = ImColor(0, 0, 0, 0);
	mFrameGraphOption.mDefaultSlotColor = ImColor(15, 100, 200, 255);
	mFrameGraphOption.mNodeSlotRadius = 10;
	mFrameGraphOption.mRounding = 0;
	mFrameGraphOption.mMinZoom = 0.1f;
	mFrameGraphOption.mMaxZoom = 10.0f;;
	setupPaintFunctor([this]() {
		QCamera* camera = mRenderer->getCamera();
		if (camera) {
			auto& io = ImGui::GetIO();
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
			ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, 40));
			ImGui::Begin("ViewportBar", 0, mViewportBarFlags);
			ImGui::Checkbox("LineMode", &bLineMode);
			ImGui::SameLine(); ImGui::RadioButton("translate", (int*)&mOperation, ImGuizmo::OPERATION::TRANSLATE);
			ImGui::SameLine(); ImGui::RadioButton("rotate", (int*)&mOperation, ImGuizmo::OPERATION::ROTATE);
			ImGui::SameLine(); ImGui::RadioButton("scale", (int*)&mOperation, ImGuizmo::OPERATION::SCALE);
			ImGui::SameLine(); ImGui::SliderFloat("camera speed", &camera->getMoveSpeedRef(), 0.01, 2);
			ImGui::SameLine(); ImGui::Checkbox("FrameGraph", &bShowFrameGraph);
			ImGui::End();

			if (bLineMode) {
				QRhiGraphicsPipelineBuilder::setPolygonModeOverride(QRhiGraphicsPipeline::Line);
			}
			else {
				QRhiGraphicsPipelineBuilder::clearPolygonModeOverride();
			}
			if (bShowFrameGraph) {

				ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(0, viewport->WorkSize.y - ImGui::GetFontSize()), ImColor(0, 255, 0, 255), QString("FPS: %1").arg("").toLocal8Bit().data());

				ImGui::SetNextWindowPos(ImVec2(0, 40));
				ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - 40));

				ImGui::Begin("Frame Graph", NULL, mViewportBarFlags);
				GraphEditor::Show(mFrameGraphDelegate, mFrameGraphOption, mFrameGraphViewState, true, &mFitOnScreen);
				ImGui::End();
			}

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
	mFrameGraphDelegate.rebuild(mRenderer->getFrameGarph());
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
				mFitOnScreen = GraphEditor::Fit_SelectedNodes;
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

