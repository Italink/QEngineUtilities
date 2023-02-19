#include "Utils/ImGuiWidgets.h"
#include "imgui_internal.h"

static inline ImVec2 operator*(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x * rhs, lhs.y * rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs) { return ImVec2(lhs.x / rhs, lhs.y / rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x / rhs.x, lhs.y / rhs.y); }

bool ImGui::IconButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2& size, const char* label, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& padding, const ImVec4& bg_col, const ImVec4& tint_col) {
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;
	const int spacing = 5;
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect iconRect(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
	if (label_size.x > 0) {
		bb.Max = bb.Max + ImVec2(label_size.x + 5, 0);
	}
	ItemSize(bb);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);
	if (bg_col.w > 0.0f)
		window->DrawList->AddRectFilled(bb.Min + padding, bb.Max - padding, GetColorU32(bg_col));
	window->DrawList->AddImage(texture_id, iconRect.Min + padding, iconRect.Max - padding, uv0, uv1, GetColorU32(tint_col));
	
	if (label_size.x > 0) {
		const ImGuiStyle& style = g.Style;
		RenderTextClipped(bb.Min + style.FramePadding + ImVec2(size.x + 5, 0), bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	}
	return pressed;
}

bool ImGui::IconButton(ImTextureID user_texture_id, const ImVec2& size, const char* label, const ImVec4& tint_col, int frame_padding, const ImVec2& uv0, const ImVec2& uv1,  const ImVec4& bg_col) {
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	if (window->SkipItems)
		return false;

	// Default to using texture ID as ID. User can still push string/integer prefixes.
	PushID((void*)(intptr_t)user_texture_id);
	const ImGuiID id = window->GetID("#image");
	PopID();

	const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : g.Style.FramePadding;
	return IconButtonEx(id, user_texture_id, size,label, uv0, uv1, padding, bg_col, tint_col);
}

