#include "python.h"

#include "core/utilities.h"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/embed.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>
#include <mutex>

namespace py = pybind11;
namespace fs = std::filesystem;

struct PythonScript
{
	int ID = -1;
	bool bFile = false;
	std::string Code = "";
	std::filesystem::path File;
};

template<typename T>
class ThreadSafeQueue
{
public:
	bool Pop(T& Element)
	{
		std::lock_guard<std::mutex> guard(Mutex);

		if (!Queue.empty())
		{
			Element = Queue.front();
			Queue.pop();
			return true;
		}

		return false;
	}

	void Push(const T& Element)
	{
		std::lock_guard<std::mutex> guard(Mutex);
		Queue.push(Element);
	}

protected:
	std::mutex Mutex;
	std::queue<T> Queue;
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
				Execute(Script);
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
			Script.Code = "";

			try
			{
				if (!std::filesystem::exists(Script.File))
					return Respond(Script.ID, PythonScriptError::FilePathInvalid);

				std::ifstream InputFileStream(Script.File.c_str());
				if (InputFileStream && InputFileStream.is_open())
					Script.Code.assign((std::istreambuf_iterator<char>(InputFileStream)), std::istreambuf_iterator< char >());
				else
					return Respond(Script.ID, PythonScriptError::InputFileStreamFailed);
			}
			catch (const std::exception& e)
			{
				return Respond(Script.ID, PythonScriptError::FileLoadException, e);
			}
			catch (...)
			{
				return Respond(Script.ID, PythonScriptError::FileLoadException, std::exception("Unknown exception catch (...)"));
			}
		}

		return ExecuteInternal(Script);
	}

	PythonScriptError ExecuteInternal(PythonScript& Script)
	{
		try
		{
			py::exec(Script.Code);
			return Respond(Script.ID, PythonScriptError::None);
		}
		catch (const py::error_already_set& e)
		{
			// TODO: pyerrors.h
			//if (e.matches(PyExc_FileNotFoundError))

			std::string py_error("Exception thrown in python interpreter:\n");
			py_error += e.what();

			return Respond(Script.ID, PythonScriptError::PybindException, std::exception(py_error.c_str()));
		}
		catch (const std::exception& e)
		{
			return Respond(Script.ID, PythonScriptError::ExecuteException, e);
		}
		catch (...)
		{
			// TODO: Handle all python exceptions listed here https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html
			return Respond(Script.ID, PythonScriptError::ExecuteException, std::exception("Unknown exception catch (...)"));
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

void PythonInterpreter::Execute(const std::string& Code, int ScriptID)
{
	PythonScript Script;
	Script.ID = ScriptID;
	Script.bFile = false;
	Script.Code = Code;
	Thread.ScriptExecutionQueue.Push(Script);
}

void PythonInterpreter::Execute(const std::filesystem::path& FilePath, int ScriptID)
{
	PythonScript Script;
	Script.ID = ScriptID;
	Script.bFile = true;
	Script.File = FilePath;
	Thread.ScriptExecutionQueue.Push(Script);
}

bool PythonInterpreter::PopScriptResponse(ScriptExecutionResponse& Response)
{
	return Thread.ScriptExecutionResponses.Pop(Response);
}
