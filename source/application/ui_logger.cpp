#include "ui_logger.h"

#include "ui_core.h"
#include "imgui_internal.h"

void UILogger::Initialize()
{

}

void UILogger::Shutdown()
{
	Clear();
}

void UILogger::Clear()
{
	logString.clear();
}

void UILogger::AddLog(const char* fmt)
{
	logString.append(fmt);
}

void UILogger::Draw(const char* title, bool* p_open)
{
	if (!ImGui::Begin(title, p_open))
	{
		ImGui::End();
		return;
	}

	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		Clear();
	}

	ImGui::SameLine();
	if (ImGui::Button("Copy"))
	{
		ImGui::LogToClipboard();
	}
		
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	if (ImGui::BeginListBox("##ScrollingRegion", ImVec2(-FLT_MIN, -FLT_MIN)))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		ImGui::TextUnformatted(logString.c_str());

		if (ImGui::GetActiveID() == ImGui::GetWindowScrollbarID(ImGui::GetCurrentWindow(), ImGuiAxis_Y))
		{
			autoScrollToBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();
		}
		else if (autoScrollToBottom)
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::PopStyleVar();
		ImGui::EndListBox();
	}
	ImGui::PopStyleVar();
	ImGui::End();
}
