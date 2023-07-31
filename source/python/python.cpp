#include "python.h"

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

std::atomic<ScriptId> PythonInterpreter::activeScriptId = -1;

struct PythonScript
{
	ScriptId id = INVALID_SCRIPT_ID;
	bool bFile = false;
	std::string code = "";
	std::filesystem::path file;
};

class PythonThread
{
public:
	std::atomic<bool> bRunning = false;
	std::thread Thread;

	ThreadSafeQueue<PythonScript> ScriptExecutionQueue;
	ThreadSafeQueue<ScriptExecutionResponse> ScriptExecutionResponses;

	void Loop()
	{
		// see py::scoped_interpreter
		py::initialize_interpreter(true, 0, nullptr, true);

		while (bRunning)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			PythonScript Script;
			if (ScriptExecutionQueue.Pop(Script))
			{
				PythonInterpreter::activeScriptId = Script.id;
				Execute(Script);
				PythonInterpreter::activeScriptId = INVALID_SCRIPT_ID;
			}
		}

		py::finalize_interpreter();
	}

	void Start()
	{
		assert("Only one interpreter instance is allowed" && !bRunning);

		bRunning = true;
		Thread = std::thread(&PythonThread::Loop, this);
	}

	void Stop()
	{
		if (bRunning)
		{
			bRunning = false;
			if (Thread.joinable())
			{
				Thread.join();
			}
			Thread = std::thread();
		}
	}

protected:
	PythonScriptError Respond(int ID, PythonScriptError Response)
	{
		return Respond(ID, Response, std::exception(""));
	}

	PythonScriptError Respond(int ID, PythonScriptError Response, const std::exception& e)
	{
		ScriptExecutionResponses.Push({ ID, Response, e });
		return Response;
	}

	PythonScriptError Execute(PythonScript& Script)
	{
		if (Script.bFile)
		{
			Script.code = "";

			try
			{
				if (!std::filesystem::exists(Script.file))
					return Respond(Script.id, PythonScriptError::FilePathInvalid);

				std::ifstream InputFileStream(Script.file.c_str());
				if (InputFileStream && InputFileStream.is_open())
					Script.code.assign((std::istreambuf_iterator<char>(InputFileStream)), std::istreambuf_iterator< char >());
				else
					return Respond(Script.id, PythonScriptError::InputFileStreamFailed);
			}
			catch (const std::exception& e)
			{
				return Respond(Script.id, PythonScriptError::FileLoadException, e);
			}
			catch (...)
			{
				return Respond(Script.id, PythonScriptError::FileLoadException, std::exception("Unknown exception catch (...)"));
			}
		}

		return ExecuteInternal(Script);
	}

	PythonScriptError ExecuteInternal(PythonScript& Script)
	{
		try
		{
			py::exec(Script.code);
			return Respond(Script.id, PythonScriptError::None);
		}
		catch (const py::error_already_set& e)
		{
			// TODO: pyerrors.h
			//if (e.matches(PyExc_FileNotFoundError))

			std::string py_error("Exception thrown in python interpreter:\n");
			py_error += e.what();

			return Respond(Script.id, PythonScriptError::PybindException, std::exception(py_error.c_str()));
		}
		catch (const std::exception& e)
		{
			return Respond(Script.id, PythonScriptError::ExecuteException, e);
		}
		catch (...)
		{
			// TODO: Handle all python exceptions listed here https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html
			return Respond(Script.id, PythonScriptError::ExecuteException, std::exception("Unknown exception catch (...)"));
		}
	}
};

PythonThread Thread;

void PythonInterpreter::Initialize()
{
	Thread.Start();
}

void PythonInterpreter::Shutdown()
{
	Thread.Stop();
}

void PythonInterpreter::Execute(const std::string& code, ScriptId id)
{
	PythonScript script;
	script.id = id;
	script.bFile = false;
	script.code = code;
	Thread.ScriptExecutionQueue.Push(script);
}

void PythonInterpreter::Execute(const std::filesystem::path& filePath, ScriptId id)
{
	PythonScript script;
	script.id = id;
	script.bFile = true;
	script.file = filePath;
	Thread.ScriptExecutionQueue.Push(script);
}

bool PythonInterpreter::PopScriptResponse(ScriptExecutionResponse& response)
{
	return Thread.ScriptExecutionResponses.Pop(response);
}
