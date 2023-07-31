#pragma once

#include <filesystem>
#include <atomic>

enum class PythonScriptError
{
	None = 0,
	FileLoadException,
	FilePathInvalid,
	InputFileStreamFailed,
	ExecuteException,
	PybindException
};

typedef int ScriptId;
#define INVALID_SCRIPT_ID -1

struct ScriptExecutionResponse
{
	ScriptId id = INVALID_SCRIPT_ID;
	PythonScriptError error = PythonScriptError::None;
	std::exception exception;
};

class PythonInterpreter
{
public:
	static std::atomic<ScriptId> activeScriptId;

	PythonInterpreter() {}
	~PythonInterpreter() {}

	void Initialize();
	void Shutdown();

	void Execute(const std::string& code, ScriptId id = INVALID_SCRIPT_ID);
	void Execute(const std::filesystem::path& filePath, ScriptId id = INVALID_SCRIPT_ID);

	bool PopScriptResponse(ScriptExecutionResponse& response);
};
