#ifndef ImGuiWidgets_h__
#define ImGuiWidgets_h__

#include "imgui_internal.h"

namespace ImGui {
bool IconButton(ImTextureID user_texture_id, const ImVec2& icon_size , const char* label, const ImVec4& tint_col = ImVec4(1, 1, 1, 1), int frame_padding = -1, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& bg_col = ImVec4(0, 0, 0, 0));
bool IconButtonEx(ImGuiID id, ImTextureID texture_id, const ImVec2& size, const char* label, const ImVec2& uv0, const ImVec2& uv1, const ImVec2& padding, const ImVec4& bg_col, const ImVec4& tint_col);
}

#endif // ImGuiWidgets_h__
