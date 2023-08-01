#include "ui_layout_ouliner.h"
#include "ui_core.h"

namespace UI
{
	void DisplayOutliner_Tree(const WeakPtr<Object> weakObject)
	{
		static const ImTextureID checkbox_enabled = (ImTextureID)(intptr_t)2;
		static const ImTextureID checkbox_disabled = (ImTextureID)(intptr_t)1;
		static const ImTextureID paperbin = (ImTextureID)(intptr_t)5;

		Object* object = weakObject.Get();
		if (!object) return;

		ImGui::PushID(object->Id());

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
				ImGui::Text(ICON_FA_CAMERA "  Camera");
				ImGui::SameLine(0, 2);
				ImGui::Text(std::to_string(component.As<Camera>()->Id()).c_str());
				break;
			}
			case ObjectType_GLTriangleMesh:
			{
				ImGui::Text(ICON_FA_OBJECT_GROUP "  Mesh");
				ImGui::SameLine(0, 2);
				ImGui::Text("");
				break;
			}
			case ObjectType_GLLine:
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
