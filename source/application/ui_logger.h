#pragma once

#include <string>
#include <vector>

typedef size_t UILoggerId;

#define LOG_STDOUT 0
#define LOG_NET 0
#define LOG_NET_SEND 1
#define LOG_NET_RECEIVE 2

#define RegisterLogger(loggerid, name) UILoggerId loggerid = App::ui.logging.Register(name)
#define Log(loggerid, str) App::ui.logging.AddLog(loggerid, str)
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

	void AddLog(const char* message);

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

	void AddLog(UILoggerId loggerid, const char* message);

	UILogger* Get(UILoggerId loggerid) { return loggers[loggerid]; }

	void Draw(bool* p_open = NULL, bool bSeparateWindow = false);
};
