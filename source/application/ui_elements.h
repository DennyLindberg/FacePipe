#pragma once

#include "imgui.h"

namespace ImGui
{
	void ImageCheckbox(const char* str_id, bool* property, ImTextureID enabled_image, ImTextureID disabled_image, ImVec2 padding = ImVec2{ 0,0 });
}

