#include "python.h"

#include "core/utilities.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/embed.h>

#include <iostream>
#include <filesystem>
#include <fstream>

namespace py = pybind11;
namespace fs = std::filesystem;

bool PythonInterpreter::bInterpreterInitialized = false;

PythonInterpreter::PythonInterpreter()
{
	assert("Only one interpreter instance is allowed" && !PythonInterpreter::bInterpreterInitialized);
	PythonInterpreter::bInterpreterInitialized = true;

	// see py::scoped_interpreter
	py::initialize_interpreter(true, 0, nullptr, true);
}

PythonInterpreter::~PythonInterpreter()
{
	py::finalize_interpreter();
	PythonInterpreter::bInterpreterInitialized = false;
}

PythonScriptError PythonInterpreter::Execute(PythonScript& Script)
{
	LastException = std::exception();

	if (Script.bFile)
	{
		Script.Code = "";

		try
		{
			if (!std::filesystem::exists(Script.File))
				return PythonScriptError::FilePathInvalid;

			std::ifstream InputFileStream(Script.File.c_str());
			if (InputFileStream && InputFileStream.is_open())
				Script.Code.assign((std::istreambuf_iterator<char>(InputFileStream)), std::istreambuf_iterator< char >());
			else
				return PythonScriptError::InputFileStreamFailed;
		}
		catch (const std::exception& e)
		{
			LastException = e;
			return PythonScriptError::FileLoadException;
		}
		catch (...)
		{
			LastException = std::exception("Unknown exception catch (...)");
			return PythonScriptError::FileLoadException;
		}
	}

	return ExecuteInternal(Script);
}

PythonScriptError PythonInterpreter::ExecuteInternal(PythonScript& Script)
{
	try
	{
		py::exec(Script.Code);
		return PythonScriptError::None;
	}
	catch (const py::error_already_set& e)
	{
		// TODO: pyerrors.h
		//if (e.matches(PyExc_FileNotFoundError))

		std::string py_error("Exception thrown in python interpreter:\n");
		py_error += e.what();

		LastException = std::exception(py_error.c_str());
		
		return PythonScriptError::PybindException;
	}
	catch (const std::exception& e)
	{
		LastException = e;
		return PythonScriptError::ExecuteException;
	}
	catch (...)
	{
		// TODO: Handle all python exceptions listed here https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html
		LastException = std::exception("Unknown exception catch (...)");
		return PythonScriptError::ExecuteException;
	}
}