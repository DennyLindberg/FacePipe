#pragma once

#include "core/core.h"

#if PYTHON_ENABLED
#if PYTHON_MULTITHREADED
#include "python/python_threaded.h"
#else
#include "python/python.h"
#endif

class Scripting
{
public:
	Scripting() {}
	~Scripting() {}

#if PYTHON_MULTITHREADED
	PythonInterpreterThreaded python;
#else
	PythonInterpreter python;
#endif

	void Initialize();
	void Shutdown();
	void Tick();

	bool Execute(const std::string& code, ScriptId id = INVALID_SCRIPT_ID);
	bool Execute(const std::filesystem::path& filePath, ScriptId id = INVALID_SCRIPT_ID);

	std::vector<std::string> scriptReturnData;
};

#else
class Scripting
{
public:
	Scripting() {}
	~Scripting() {}

	void Initialize() {}
	void Shutdown() {}
	void Tick() {}

	bool Execute(const std::string& code, ScriptId id = INVALID_SCRIPT_ID);
	bool Execute(const std::filesystem::path& filePath, ScriptId id = INVALID_SCRIPT_ID);
};
#endif