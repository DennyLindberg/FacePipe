#pragma once

#include <string>
#include <vector>

typedef size_t UILoggerId;

#define LOG_STDOUT 0

#define RegisterLogger(loggerid, name) UILoggerId loggerid = App::ui.logging.Register(name)
#define Log(loggerid, str) App::ui.logging.Get(loggerid)->AddLog(str)
#define Logf(loggerid, str, ...) Log(loggerid, std::format(str, __VA_ARGS__).c_str())

class UILogger
{
public:
	std::string name;
	std::string logString;
	bool autoScrollToBottom = true;

public:
	void Initialize();
	void Shutdown();

	void Clear();

	void AddLog(const char* fmt);

	void Draw(bool* p_open = NULL);
};

class UILoggerManager
{
protected:
	std::vector<UILogger*> loggers;
	UILoggerId activeUiLogger = LOG_STDOUT;

public:
	void Initialize();
	void Shutdown();

	void Tick();

	void Register(UILoggerId loggerid, const char* name);

	UILoggerId Register(const char* name);

	const std::vector<UILogger*>& GetLoggers() const { return loggers; }

	UILogger* Get(UILoggerId loggerid) { return loggers[loggerid]; }

	void Draw(bool* p_open = NULL);
};
