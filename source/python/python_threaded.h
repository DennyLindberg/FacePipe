#pragma once

#include <filesystem>
#include <atomic>
#include "python.h"

struct ScriptExecutionResponse
{
	ScriptId id = INVALID_SCRIPT_ID;
	PythonScriptError error = PythonScriptError::None;
	std::exception exception;
};

class PythonInterpreterThreaded
{
public:
	static std::atomic<ScriptId> activeScriptId;

	PythonInterpreterThreaded() {}
	~PythonInterpreterThreaded() {}

	void Initialize();
	void Shutdown();

	void Execute(const std::string& code, ScriptId id = INVALID_SCRIPT_ID);
	void Execute(const std::filesystem::path& filePath, ScriptId id = INVALID_SCRIPT_ID);

	bool PopScriptResponse(ScriptExecutionResponse& response);
};
