#include "ui_elements.h"

#include "ui_core.h"
#include "viewport.h"

#include "imgui_internal.h"

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

	void GetWindowContentRegionScreenSpace(ImVec2& min, ImVec2& max)
	{
		ImVec2 wPos = ImGui::GetWindowPos();
		min = ImGui::GetWindowContentRegionMin() + wPos;
		max = ImGui::GetWindowContentRegionMax() + wPos;
	}

	void DebugDrawWindowContentRegion()
	{
		ImVec2 min, max;
		ImGui::GetWindowContentRegionScreenSpace(min, max);
		ImGui::GetWindowDrawList()->AddRect(min, max, ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)), 0.0f, 0, 1.0f);
	}

	void DrawWindowContentRegionFilled(const ImVec4 color)
	{
		ImVec2 min, max;
		ImGui::GetWindowContentRegionScreenSpace(min, max);
		ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(color), 0.0f, 0);
	}

	void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void DrawViewport(UIManager* ui, Viewport* viewport)
	{
		GLuint Texture, TextureWidth, TextureHeight;
		if (GLFramebuffers::GetTexture(viewport->framebuffer, Texture, TextureWidth, TextureHeight))
		{
			float availWidth = ImGui::GetContentRegionAvail().x;

			if (availWidth < 16.0f)
				availWidth = 16.0f;

			if (availWidth > App::settings.windowWidth)
				availWidth = (float) App::settings.windowWidth;

			TextureWidth = (GLuint)(availWidth);
			TextureHeight = (GLuint)(availWidth/App::settings.WindowRatio());

			viewport->Resize(TextureWidth, TextureHeight);

			ImGui::Image((ImTextureID)(intptr_t)Texture, ImVec2((float)TextureWidth, (float)TextureHeight), { 0, 1 }, { 1, 0 });
			ui->UpdateActiveViewport(ui->previewViewport, ImGui::IsItemHovered());
		}
	}

	void DrawModal(const char* label, ImVec2 modalSize, std::function<void()> modalContents)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));
		if (ImGui::Begin(label, NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::PopStyleVar();

			modalContents();
		}
		ImGui::PopStyleColor();
		ImGui::End();
	}

	void OnPopupModalSave(const char* label, const char* header, const char* message, const std::vector<const char*>& buttons, std::function<void(const char*)> callback_fun)
	{
		if (ImGui::BeginPopupModal("SaveChanges?", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::Text(ICON_FA_FLOPPY_DISK" (?)");
			ImGui::Separator();

			ImGui::Text("");

			ImGuiStyle& style = ImGui::GetStyle();
			float size = ImGui::CalcTextSize(message).x + style.FramePadding.x * 2.0f;
			float avail = ImGui::GetContentRegionAvail().x;

			float off = (avail - size) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			ImGui::Text(message);

			ImGui::Text("");

			float spacing = 10.0f;
			int count = (int)buttons.size();
			size = style.FramePadding.x*count + spacing*count;
			for (const char* message : buttons)
			{
				size += ImGui::CalcTextSize(message).x;
			}

			off = (avail - size) * 0.5f;
			if (off > 0.0f)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

			for (const char* message : buttons)
			{
				if (ImGui::Button(message))
				{
					callback_fun(message);
				}
				ImGui::SameLine(0.0f, spacing);
			}

			ImGui::EndPopup();
		}
	}
}
