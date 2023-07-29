#pragma once

#include <filesystem>

enum class PythonScriptError
{
	None = 0,
	FileLoadException,
	FilePathInvalid,
	InputFileStreamFailed,
	ExecuteException,
	PybindException
};

struct ScriptExecutionResponse
{
	int ScriptID = 0;
	PythonScriptError Error = PythonScriptError::None;
	std::exception Exception;
};

class PythonInterpreter
{
public:
	PythonInterpreter() {}
	~PythonInterpreter() {}

	void Initialize();
	void Shutdown();

	void Execute(const std::string& Code, int ScriptID = -1);
	void Execute(const std::filesystem::path& FilePath, int ScriptID = -1);

	bool PopScriptResponse(ScriptExecutionResponse& Response);
};
