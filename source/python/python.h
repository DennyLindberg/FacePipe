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

struct PythonScript
{
	ScriptId id = INVALID_SCRIPT_ID;
	bool bFile = false;
	std::string code = "";
	std::filesystem::path file;

	PythonScriptError response = PythonScriptError::None;
	std::exception exception;
	std::string returnValue = "";
};

class PythonInterpreter
{
protected:
	std::vector<PythonScript> activeScripts;

public:
	static ScriptId activeScriptId;

	PythonInterpreter() {}
	~PythonInterpreter() {}

	void Initialize();
	void Shutdown();

	void Tick();

	void PushScript(const PythonScript& script);
	bool Execute(const std::string& code, ScriptId id = INVALID_SCRIPT_ID);
	bool Execute(const std::filesystem::path& filePath, ScriptId id = INVALID_SCRIPT_ID);
	bool Execute(PythonScript& script);
};
