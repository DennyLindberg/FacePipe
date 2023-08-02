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
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	if (!ImGui::Begin(title, p_open))
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}
	ImGui::PopStyleVar();

	ImGui::SameLine();
	if (ImGui::Button("Clear"))
	{
		Clear();
	}

	ImGui::SameLine();
	if (ImGui::Button("Copy"))
	{
		ImGui::SetClipboardText(logString.c_str());
	}

	ImGui::Separator();

	const float lineHeight = ImGui::GetTextLineHeightWithSpacing();
	ImGui::BeginChild("##ScrollingRegion", ImVec2(-FLT_MIN, -FLT_MIN), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_HorizontalScrollbar);
	{
		ImGui::PushStyleColor(ImGuiCol_FrameBg, { 0.f, 0.f, 0.f, 0.f }); // remove text input box

		static const char* textid = "##logtext";
		ImGui::InputTextMultiline(
			textid,
			const_cast<char*>(logString.c_str()), // ugly const cast
			logString.size() + 1, // needs to include '\0'
			ImVec2(-FLT_MIN, -FLT_MIN),
			ImGuiInputTextFlags_ReadOnly
		);

		ImGui::BeginChild(textid);
		if (ImGui::GetActiveID() || ImGui::GetScrollMaxY() == 0.0f)
		{
			autoScrollToBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();
		}
		else if (autoScrollToBottom)
		{
			ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();

		ImGui::PopStyleColor();

		ImGui::EndChild();
	}
	ImGui::End();
}
