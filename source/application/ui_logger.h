#pragma once

#include <string>

class UILogger
{
public:
	std::string logString;
	bool autoScrollToBottom = true;

public:
	void Initialize();
	void Shutdown();

	void Clear();

	void AddLog(const char* fmt);

	void Draw(const char* title, bool* p_open = NULL);
};