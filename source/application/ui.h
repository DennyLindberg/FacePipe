#pragma once

#include "core/object.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imnodes.h"

#include "IconsFontAwesome6.h"

class UIManager
{
public:
	void Initialize();
	void Shutdown();
};

namespace ImGui
{
	void ImageCheckbox(const char* str_id, bool* property, ImTextureID enabled_image, ImTextureID disabled_image, ImVec2 padding = ImVec2{0,0});
}

namespace UI
{
	enum class Icon
	{
		test = 0xF083
	};

	void DisplayOutliner(const WeakPtr<Object> weakObject);
	void DisplaySelectionDetails(const WeakPtr<Object> selectedObject);
}
