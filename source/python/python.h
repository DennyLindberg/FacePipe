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

class PythonScript
{
public:
	friend class PythonInterpreter;

	PythonScript(const std::string& CodeString)
		: bFile(false), Code(CodeString)
	{}

	PythonScript(const std::filesystem::path& FilePath)
		: bFile(true), File(FilePath)
	{}

protected:
	bool bFile = false;
	std::string Code = "";
	std::filesystem::path File;
};

class PythonInterpreter
{
public:
	PythonInterpreter();
	~PythonInterpreter();

	PythonScriptError Execute(PythonScript& Script);
	const std::exception& GetLastException() const { return LastException; }

protected:
	PythonScriptError ExecuteInternal(PythonScript& Script);

	static bool bInterpreterInitialized;
	std::exception LastException;
};
