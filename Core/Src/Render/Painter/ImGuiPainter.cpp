#include "ImGuiPainter.h"
#include "QApplication"
#include "qevent.h"
#include "QGuiApplication"
#include "QClipboard"
#include "QDateTime"


const int64_t IMGUI_VERTEX_BUFFER_SIZE = 50000;
const int64_t IMGUI_INDEX_BUFFER_SIZE = 50000;

const QHash<int, ImGuiKey> keyMap = {
	{ Qt::Key_Tab, ImGuiKey_Tab },
	{ Qt::Key_Left, ImGuiKey_LeftArrow },
	{ Qt::Key_Right, ImGuiKey_RightArrow },
	{ Qt::Key_Up, ImGuiKey_UpArrow },
	{ Qt::Key_Down, ImGuiKey_DownArrow },
	{ Qt::Key_PageUp, ImGuiKey_PageUp },
	{ Qt::Key_PageDown, ImGuiKey_PageDown },
	{ Qt::Key_Home, ImGuiKey_Home },
	{ Qt::Key_End, ImGuiKey_End },
	{ Qt::Key_Insert, ImGuiKey_Insert },
	{ Qt::Key_Delete, ImGuiKey_Delete },
	{ Qt::Key_Backspace, ImGuiKey_Backspace },
	{ Qt::Key_Space, ImGuiKey_Space },
	{ Qt::Key_Enter, ImGuiKey_Enter },
	{ Qt::Key_Return, ImGuiKey_Enter },
	{ Qt::Key_Escape, ImGuiKey_Escape },
	{ Qt::Key_A, ImGuiKey_A },
	{ Qt::Key_C, ImGuiKey_C },
	{ Qt::Key_V, ImGuiKey_V },
	{ Qt::Key_X, ImGuiKey_X },
	{ Qt::Key_Y, ImGuiKey_Y },
	{ Qt::Key_Z, ImGuiKey_Z },
	{ Qt::MiddleButton, ImGuiMouseButton_Middle }
};

const QHash<ImGuiMouseCursor, Qt::CursorShape> cursorMap = {
	{ ImGuiMouseCursor_Arrow,      Qt::CursorShape::ArrowCursor },
	{ ImGuiMouseCursor_TextInput,  Qt::CursorShape::IBeamCursor },
	{ ImGuiMouseCursor_ResizeAll,  Qt::CursorShape::SizeAllCursor },
	{ ImGuiMouseCursor_ResizeNS,   Qt::CursorShape::SizeVerCursor },
	{ ImGuiMouseCursor_ResizeEW,   Qt::CursorShape::SizeHorCursor },
	{ ImGuiMouseCursor_ResizeNESW, Qt::CursorShape::SizeBDiagCursor },
	{ ImGuiMouseCursor_ResizeNWSE, Qt::CursorShape::SizeFDiagCursor },
	{ ImGuiMouseCursor_Hand,       Qt::CursorShape::PointingHandCursor },
	{ ImGuiMouseCursor_NotAllowed, Qt::CursorShape::ForbiddenCursor },
};

QByteArray g_currentClipboardText;

void embraceTheDarkness() {
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(8.00f, 8.00f);
	style.FramePadding = ImVec2(5.00f, 2.00f);
	style.CellPadding = ImVec2(6.00f, 6.00f);
	style.ItemSpacing = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
	style.IndentSpacing = 25;
	style.ScrollbarSize = 15;
	style.GrabMinSize = 10;
	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = 1;
	style.TabBorderSize = 1;
	style.WindowRounding = 7;
	style.ChildRounding = 4;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ScrollbarRounding = 9;
	style.GrabRounding = 3;
	style.LogSliderDeadzone = 4;
	style.TabRounding = 4;
}

ImGuiPainter::ImGuiPainter()
{
	mImGuiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(mImGuiContext);

	//embraceTheDarkness();
	ImGuiIO& io = ImGui::GetIO();

	//io.Fonts->AddFontFromFileTTF(R"(E:\ModernGraphicsEngineGuide\Source\QEngineUtilities\Editor\Resources\DroidSans.ttf)", 16);
	//io.Fonts->Build();

	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)
	io.FontGlobalScale = qApp->devicePixelRatio();
	io.BackendPlatformName = "Qt ImGUI";
	io.SetClipboardTextFn = [](void* user_data, const char* text) {
		Q_UNUSED(user_data);
		QGuiApplication::clipboard()->setText(text);
	};

	io.GetClipboardTextFn = [](void* user_data) {
		Q_UNUSED(user_data);
		g_currentClipboardText = QGuiApplication::clipboard()->text().toUtf8();
		return (const char*)g_currentClipboardText.data();
	};
}

void ImGuiPainter::setupWindow(QWindow* window)
{
	mWindow = window;
	mWindow->installEventFilter(this);
}



void ImGuiPainter::compile() {
	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer, sizeof(ImDrawVert) * IMGUI_VERTEX_BUFFER_SIZE));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::IndexBuffer, sizeof(ImDrawIdx) * IMGUI_INDEX_BUFFER_SIZE));
	mIndexBuffer->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QMatrix4x4)));
	mUniformBuffer->create();

	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
				   QRhiSampler::Linear,
				   QRhiSampler::None,
				   QRhiSampler::ClampToEdge,
				   QRhiSampler::ClampToEdge));
	mSampler->create();

	unsigned char* pixels;
	int width, height;
	ImGui::SetCurrentContext(mImGuiContext);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	mFontImage = QImage(pixels, width, height, QImage::Format::Format_RGBA8888);
	mFontTexture.reset(mRhi->newTexture(QRhiTexture::RGBA8, QSize(width, height), 1));
	mFontTexture->create();
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.srcColor = QRhiGraphicsPipeline::SrcAlpha;
	blendState.enable = true;
	mPipeline->setTargetBlends({ blendState });
	mPipeline->setFlags(QRhiGraphicsPipeline::UsesScissor);
	mPipeline->setSampleCount(mSampleCount);
	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, R"(#version 440
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0)out vec2 vUV;
layout(location = 1)out vec4 vColor;
layout(std140,binding = 0) uniform buf{
	mat4 mvp;
}ubuf;

out gl_PerVertex { vec4 gl_Position; };

void main(){
	vUV = inUV;
	vColor = inColor;
	gl_Position = ubuf.mvp*vec4(inPosition,0,1);
}
)");

	QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) in vec2 vUV;
layout(location = 1) in vec4 vColor;
layout(binding = 1) uniform sampler2D uTexture;
layout(location = 0) out vec4 OutColor;
void main(){
	OutColor = vColor * texture(uTexture,vUV);
}
)");

	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
	});
	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage,mUniformBuffer.get()),
		QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,mFontTexture.get(),mSampler.get())
	});
	mBindings->create();

	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(ImDrawVert)) });
	inputLayout.setAttributes({
		QRhiVertexInputAttribute{ 0, 0, QRhiVertexInputAttribute::Float2, offsetof(ImDrawVert, pos) },
		QRhiVertexInputAttribute{ 0, 1, QRhiVertexInputAttribute::Float2, offsetof(ImDrawVert, uv) },
		QRhiVertexInputAttribute{ 0, 2, QRhiVertexInputAttribute::UNormByte4, offsetof(ImDrawVert, col) },
							  });
	mPipeline->setVertexInputLayout(inputLayout);
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(mRenderPassDesc);
	mPipeline->create();
}

void ImGuiPainter::resourceUpdate(QRhiResourceUpdateBatch* batch) {
	ImGui::SetCurrentContext(mImGuiContext);
	ImGuiIO& io = ImGui::GetIO();
	QSize size = mWindow->size() * mWindow->devicePixelRatio();
	io.DisplaySize = ImVec2(size.width(), size.height());
	io.DisplayFramebufferScale = ImVec2(1, 1);
	float currentTime = QDateTime::currentMSecsSinceEpoch() / 1000.0f;
	io.DeltaTime = qMax(1.0f / 60.0f, currentTime - mTime);
	qDebug() << io.DeltaTime;
	mTime = currentTime;
	if (io.WantSetMousePos) {
		const QPoint globalPos = mWindow->mapToGlobal(QPoint{ (int)io.MousePos.x, (int)io.MousePos.y });
		QCursor cursor = mWindow->cursor();
		cursor.setPos(globalPos);
		//mWindow->setCursor(cursor);
	}
	if (mWindow->isActive()) {
		const QPoint pos = mWindow->mapFromGlobal(QCursor::pos());
		io.MousePos = ImVec2(pos.x() * mWindow->devicePixelRatio(), pos.y() * mWindow->devicePixelRatio());  // Mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
	}
	else {
		io.MousePos = ImVec2(-1, -1);
	}
	for (int i = 0; i < 3; i++) {
		io.MouseDown[i] = mMousePressed[i];
	}
	io.MouseWheelH = mMouseWheelH;
	io.MouseWheel = mMouseWheel;
	mMouseWheelH = 0;
	mMouseWheel = 0;
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		return;
	const ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (io.MouseDrawCursor || (imgui_cursor == ImGuiMouseCursor_None)) {
		mWindow->setCursor(Qt::CursorShape::BlankCursor);
	}
	else {
		const auto cursor_it = cursorMap.constFind(imgui_cursor);
		if (cursor_it != cursorMap.constEnd()) {
			const Qt::CursorShape qt_cursor_shape = *(cursor_it);
			mWindow->setCursor(qt_cursor_shape);
		}
		else {
			mWindow->setCursor(Qt::CursorShape::ArrowCursor);
		}
	}
	ImGui::SetCurrentContext(mImGuiContext);
	ImGui::NewFrame();
	if (mPaintFunctor)
		mPaintFunctor();
	ImGui::EndFrame();
	ImGui::Render();

	ImDrawData* draw_data = ImGui::GetDrawData();
	int64_t vertexBufferOffset = 0;
	int64_t indexBufferOffset = 0;
	for (int i = 0; i < draw_data->CmdListsCount; i++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[i];
		batch->updateDynamicBuffer(mVertexBuffer.get(), vertexBufferOffset, cmd_list->VtxBuffer.size_in_bytes(), cmd_list->VtxBuffer.Data);
		batch->updateDynamicBuffer(mIndexBuffer.get(), indexBufferOffset, cmd_list->IdxBuffer.size_in_bytes(), cmd_list->IdxBuffer.Data);
		vertexBufferOffset += cmd_list->VtxBuffer.size_in_bytes();
		indexBufferOffset += cmd_list->IdxBuffer.size_in_bytes();
	}
	QMatrix4x4 MVP = mRhi->clipSpaceCorrMatrix();
	QRect rect(0, 0, size.width(), size.height());
	MVP.ortho(rect);
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(QMatrix4x4), MVP.constData());

	if (!mFontImage.isNull()) {
		batch->uploadTexture(mFontTexture.get(), mFontImage);
		mFontImage = {};
	}
}

void ImGuiPainter::paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) {
	ImDrawData* draw_data = ImGui::GetDrawData();
	if (!draw_data)
		return;
	int64_t vertexBufferOffset = 0;
	int64_t indexBufferOffset = 0;
	for (int i = 0; i < draw_data->CmdListsCount; i++) {
		const ImDrawList* cmd_list = draw_data->CmdLists[i];
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, renderTarget->pixelSize().width(), renderTarget->pixelSize().height()));

		const QRhiCommandBuffer::VertexInput VertexInput(mVertexBuffer.get(), vertexBufferOffset);
		cmdBuffer->setVertexInput(0, 1, &VertexInput, mIndexBuffer.get(), indexBufferOffset, sizeof(ImDrawIdx) == 2 ? QRhiCommandBuffer::IndexUInt16 : QRhiCommandBuffer::IndexUInt32);
		vertexBufferOffset += cmd_list->VtxBuffer.size_in_bytes();
		indexBufferOffset += cmd_list->IdxBuffer.size_in_bytes();
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			QRhiTexture * texPtr = static_cast<QRhiTexture*>(pcmd->GetTexID());
			if (texPtr) {
				mBindings->setBindings({
					QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage,mUniformBuffer.get()),
					QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,texPtr,mSampler.get())
									   });
				mBindings->create();
			}
			else {
				mBindings->setBindings({
					QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::VertexStage,mUniformBuffer.get()),
					QRhiShaderResourceBinding::sampledTexture(1,QRhiShaderResourceBinding::FragmentStage,mFontTexture.get(),mSampler.get())
									   });
				mBindings->create();
			}
			cmdBuffer->setShaderResources();
			if (pcmd->UserCallback) {
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else {
				QRect rect0(pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
				QRhiScissor scissor(pcmd->ClipRect.x, renderTarget->pixelSize().height() - pcmd->ClipRect.y - pcmd->ClipRect.w, pcmd->ClipRect.z, pcmd->ClipRect.w);
				QRect rect1(pcmd->ClipRect.x, pcmd->ClipRect.y, pcmd->ClipRect.z, pcmd->ClipRect.w);
				cmdBuffer->setScissor(scissor);
				cmdBuffer->drawIndexed(pcmd->ElemCount, 1, pcmd->IdxOffset, pcmd->VtxOffset, 0);
			}
		}
	}
}

bool ImGuiPainter::eventFilter(QObject* watched, QEvent* event)
{
	if (watched != nullptr) {
		switch (event->type()) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease: {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			mMousePressed[0] = mouseEvent->buttons() & Qt::LeftButton;
			mMousePressed[1] = mouseEvent->buttons() & Qt::RightButton;
			mMousePressed[2] = mouseEvent->buttons() & Qt::MiddleButton;
			break;
		}
		case QEvent::Wheel: {
			QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
			ImGui::SetCurrentContext(mImGuiContext);
			if (wheelEvent->pixelDelta().x() != 0) {
				mMouseWheelH += wheelEvent->pixelDelta().x() / (ImGui::GetTextLineHeight());
			}
			else {
				mMouseWheelH += wheelEvent->angleDelta().x() / 120;
			}
			if (wheelEvent->pixelDelta().y() != 0) {
				mMouseWheel += wheelEvent->pixelDelta().y() / (5.0 * ImGui::GetTextLineHeight());
			}
			else {
				mMouseWheel += wheelEvent->angleDelta().y() / 120;
			}
			break;
		}
		case QEvent::KeyPress:
		case QEvent::KeyRelease: {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			ImGui::SetCurrentContext(mImGuiContext);
			ImGuiIO& io = ImGui::GetIO();
			const bool key_pressed = (event->type() == QEvent::KeyPress);
			const auto key_it = keyMap.constFind(keyEvent->key());
			if (key_it != keyMap.constEnd()) {
				const int imgui_key = *(key_it);
				io.AddKeyEvent(imgui_key, key_pressed);
			}

			if (key_pressed) {
				const QString text = keyEvent->text();
				if (text.size() == 1) {
					io.AddInputCharacter(text.at(0).unicode());
				}
			}
		#ifdef Q_OS_MAC
			io.KeyCtrl = keyEvent->modifiers() & Qt::MetaModifier;
			io.KeyShift = keyEvent->modifiers() & Qt::ShiftModifier;
			io.KeyAlt = keyEvent->modifiers() & Qt::AltModifier;
			io.KeySuper = keyEvent->modifiers() & Qt::ControlModifier;
		#else
			io.KeyCtrl = keyEvent->modifiers() & Qt::ControlModifier;
			io.KeyShift = keyEvent->modifiers() & Qt::ShiftModifier;
			io.KeyAlt = keyEvent->modifiers() & Qt::AltModifier;
			io.KeySuper = keyEvent->modifiers() & Qt::MetaModifier;
		#endif
			break;
		}
		default:
			break;
		}
	}
	return QObject::eventFilter(watched, event);
}