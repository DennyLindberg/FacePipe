#include "ui.h"
#include "application.h"

void UIManager::Initialize()
{
	ImGuiIO& io = ImGui::GetIO();

	// fontawesome 6 example from https://github.com/juliettef/IconFontCppHeaders
	{
		io.Fonts->AddFontDefault();
		float baseFontSize = 16.0f; // 13.0f is the size of the default font. Change to the font size you use.
		float iconFontSize = baseFontSize * 2.0f / 3.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

		// merge in icons from Font Awesome
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config;
		icons_config.MergeMode = true;
		icons_config.PixelSnapH = true;
		icons_config.GlyphMinAdvanceX = iconFontSize;

		std::filesystem::path fontPath(App::Path("content/fonts/fontawesome/Font Awesome 6 Free-Solid-900.otf"));
		io.Fonts->AddFontFromFileTTF(fontPath.string().c_str(), iconFontSize, &icons_config, icons_ranges);
	}
}

void UIManager::Shutdown()
{

}

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

namespace UI
{
	void DisplayOutliner_Tree(const WeakPtr<Object> weakObject)
	{
		static const ImTextureID checkbox_enabled = (ImTextureID)(intptr_t)2;
		static const ImTextureID checkbox_disabled = (ImTextureID)(intptr_t)1;
		static const ImTextureID paperbin = (ImTextureID)(intptr_t)5;

		Object* object = weakObject.Get();
		if (!object) return;

		ImGui::PushID(object->GetObjectId());

		auto& children = object->GetChildren();

		// Display title (left column)
		bool bShowChildren = false;
		if (children.size() > 0)
			bShowChildren = ImGui::TreeNode(object->name.c_str());
		else
			ImGui::Text(object->name.c_str());

		// Display buttons (right column)
		ImGui::NextColumn();
		ImGui::ImageCheckbox("##test", &(object->visible), checkbox_enabled, checkbox_disabled);
		if (weakObject != App::world)
		{
			ImGui::SameLine(0, 2);
			if (ImGui::ImageButton("##delete", paperbin, ImVec2(20.0f, 20.0f)))
			{
				Object::Pool.Destroy(weakObject);
				bShowChildren = false;
			}
		}
		ImGui::NextColumn();

		// Recurse
		if (bShowChildren)
		{
			//ImGui::Indent(1.0f);
			for (const WeakPtr<Object>& child : children)
			{
				DisplayOutliner_Tree(child);
			}
			//ImGui::Unindent();
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void DisplayOutliner(const WeakPtr<Object> weakObject)
	{
		ImGui::Columns(2, "outlinercolumns");
		DisplayOutliner_Tree(App::world);
		ImGui::Columns(1);
	}

	void DisplaySelectionDetails(const WeakPtr<Object> selectedObject)
	{
		if (!selectedObject)
			return;

		ImGui::Image((ImTextureID)(intptr_t)2, ImVec2(20.0f, 20.0f));
		ImGui::SameLine(0, 2);
		ImGui::Text(selectedObject->name.c_str());

		ImGui::Indent();

		for (WeakPtrGeneric component : selectedObject->GetComponents())
		{
			switch (component.type)
			{
			case ObjectType_Camera:
			{
				ImGui::Text( ICON_FA_CAMERA "  Camera" );
				ImGui::SameLine(0, 2);
				ImGui::Text(std::to_string(component.As<Camera>()->id()).c_str());
				break;
			}
			case ObjectType_Mesh:
			{
				ImGui::Text( ICON_FA_OBJECT_GROUP "  Mesh" );
				ImGui::SameLine(0, 2);
				ImGui::Text("");
				break;
			}
			case ObjectType_Line:
			{
				ImGui::Image((ImTextureID)(intptr_t)1, ImVec2(20.0f, 20.0f));
				ImGui::SameLine(0, 2);

				ImGui::Text("Line");
				ImGui::SameLine(0, 2);
				ImGui::Text("");
				break;
			}
			default: {}
			}
		}

		ImGui::Unindent();
	}
}
