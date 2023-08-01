#pragma once

#include "imgui.h"
#include <functional>

class Viewport;
class UIManager;

namespace ImGui
{
	void ImageCheckbox(const char* str_id, bool* property, ImTextureID enabled_image, ImTextureID disabled_image, ImVec2 padding = ImVec2{ 0,0 });
	void GetWindowContentRegionScreenSpace(ImVec2& min, ImVec2& max);
	void DebugDrawWindowContentRegion();
	void DrawWindowContentRegionFilled(const ImVec4 color);
	void HelpMarker(const char* desc); // from imgui_demo.cpp

	void DrawViewport(UIManager* ui, Viewport* viewport);

	void DrawModal(const char* label, ImVec2 modalSize, std::function<void()> modalContents);
	void DrawPopup(const char* label, const char* header, const char* message, const std::vector<const char*>& buttons, std::function<void(const char*)> callback_fun);
}
