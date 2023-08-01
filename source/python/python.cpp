#include "python.h"

#if PYTHON_ENABLED
#include "core/utilities.h"
#include "core/threads.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/embed.h>

#include <iostream>
#include <filesystem>
#include <fstream>

namespace py = pybind11;
namespace fs = std::filesystem;

ScriptId PythonInterpreter::activeScriptId = INVALID_SCRIPT_ID;

void PythonInterpreter::Initialize()
{
	activeScripts.clear();

	py::initialize_interpreter(true, 0, nullptr, true);
}

void PythonInterpreter::Shutdown()
{
	activeScripts.clear();

	py::finalize_interpreter();
}

bool ScriptError(PythonScript& script, PythonScriptError error, std::exception e = std::exception(""))
{
	script.response = error;
	script.exception = e;
	return false;
}

bool LoadScript(PythonScript& script)
{
	if (!script.bFile)
		return script.code != "";

	script.code = "";

	try
	{
		if (!std::filesystem::exists(script.file))
			return ScriptError(script, PythonScriptError::FilePathInvalid);

		std::ifstream InputFileStream(script.file.c_str());
		if (InputFileStream && InputFileStream.is_open())
			script.code.assign((std::istreambuf_iterator<char>(InputFileStream)), std::istreambuf_iterator< char >());
		else
			return ScriptError(script, PythonScriptError::InputFileStreamFailed);
	}
	catch (const std::exception& e)
	{
		return ScriptError(script, PythonScriptError::FileLoadException, e);
	}
	catch (...)
	{
		return ScriptError(script, PythonScriptError::FileLoadException, std::exception("Unknown exception catch (...)"));
	}

	return script.code != "";
}

void PythonInterpreter::Tick()
{
	for (PythonInterpreter::activeScriptId = 0; activeScriptId<activeScripts.size(); ++activeScriptId)
	{
		Execute(activeScripts[activeScriptId]);
	}
	activeScriptId = INVALID_SCRIPT_ID;
}

void PythonInterpreter::PushScript(const PythonScript& script)
{
	activeScripts.push_back(script);
}

bool ExecuteInternal(PythonScript& script)
{
	try
	{
		py::exec(script.code);
		return true;
	}
	catch (const py::error_already_set& e)
	{
		// TODO: pyerrors.h
		//if (e.matches(PyExc_FileNotFoundError))

		std::string py_error("Exception thrown in python interpreter:\n");
		py_error += e.what();

		return ScriptError(script, PythonScriptError::PybindException, std::exception(py_error.c_str()));
	}
	catch (const std::exception& e)
	{
		return ScriptError(script, PythonScriptError::ExecuteException, e);
	}
	catch (...)
	{
		// TODO: Handle all python exceptions listed here https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html
		return ScriptError(script, PythonScriptError::ExecuteException, std::exception("Unknown exception catch (...)"));
	}
}

bool PythonInterpreter::Execute(const std::string& code, ScriptId id)
{
	PythonScript script;
	script.id = id;
	script.bFile = false;
	script.code = code;
	return Execute(script);
}

bool PythonInterpreter::Execute(const std::filesystem::path& filePath, ScriptId id)
{
	PythonScript script;
	script.id = id;
	script.bFile = true;
	script.file = filePath;
	return Execute(script);
}

bool PythonInterpreter::Execute(PythonScript& script)
{
	if (!LoadScript(script))
	{
		if (script.bFile)
			std::cout << "Load script failed [" << script.id << "] [" << script.file << "] reason:\n" << script.exception.what() << std::endl;
		else
			std::cout << "Load script failed [" << script.id << "] [string]" << " reason:\n" << script.exception.what() << std::endl;
		return false;
	}

	if (!ExecuteInternal(script))
	{
		std::cout << "Execute script failed [" << script.id << "]:\n" << script.exception.what() << std::endl;
		return false;
	}

	return true;
}
#endif