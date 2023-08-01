#include "ui_elements.h"

#include "ui_core.h"

namespace ImGui
{
	void ImageCheckbox(const char* str_id, bool* property, ImTextureID enabled_image, ImTextureID disabled_image, ImVec2 padding)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, padding);

		bool bEnabled = *property;
		if (ImGui::ImageButton(str_id, bEnabled ? enabled_image : disabled_image, ImVec2(20.0f, 20.0f)))
		{
			*property = !bEnabled;
		}

		ImGui::PopStyleVar();
	}
}
