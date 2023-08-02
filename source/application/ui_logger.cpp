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

void UILogger::AddLog(const char* message)
{
	logString.append(message);
}

void UILogger::Draw(bool* p_open)
{
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
}

void UILoggerManager::Initialize()
{
	Register(LOG_STDOUT, "App");
}

void UILoggerManager::Shutdown()
{
	for (UILogger* logger : loggers)
	{
		if (logger)
		{
			logger->Shutdown();
			delete logger;
			logger = nullptr;
		}
	}
}

void UILoggerManager::Tick()
{
	Logging::Flush();

	std::string logLine;
	while (Logging::GetLine(logLine))
	{
		loggers[LOG_STDOUT]->AddLog(logLine.c_str());
	}
}

void UILoggerManager::Register(UILoggerId loggerid, const char* name)
{
	if (loggerid >= loggers.size())
	{
		loggers.resize(loggerid + 1);

		loggers[loggerid] = new UILogger();
		loggers[loggerid]->Initialize();
		loggers[loggerid]->name = std::string(name);
	}
}

UILoggerId UILoggerManager::Register(const char* name)
{
	UILoggerId id = loggers.size();
	Register(id, name);
	return id;
}

void UILoggerManager::AddLog(UILoggerId loggerid, const char* message)
{
	if (loggerid == LOG_STDOUT)
		std::cout << message; // needed so that our pipe sends to the console window - our logger will get the message once the pipe buffer is read on the thread
	else
		loggers[loggerid]->AddLog(message);
}

void UILoggerManager::Draw(bool* p_open)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	if (!ImGui::Begin("Log", p_open))
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}
	ImGui::PopStyleVar();

	UILogger* activeLogger = loggers[activeUiLogger];

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_ERASER))
		activeLogger->Clear();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Clear log");

	ImGui::SameLine();
	if (ImGui::Button(ICON_FA_COPY))
		ImGui::SetClipboardText(activeLogger->logString.c_str());
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Copy log to clipboard");

	ImGui::SameLine();
	ImGui::Text(" | ");

	for (UILoggerId i=0; i<loggers.size(); i++)
	{
		if (!loggers[i])
			continue;

		bool bIsActive = activeUiLogger == i;
		if (bIsActive)
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.15f, 0.3f, 1.0f));

		ImGui::SameLine();
		if (ImGui::Button(loggers[i]->name.c_str()))
		{
			activeUiLogger = i;
		}

		if (bIsActive)
			ImGui::PopStyleColor();
	}

	ImGui::Separator();

	activeLogger->Draw(p_open);

	ImGui::End();
}
