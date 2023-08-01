#include "scripting.h"

#include "core/threads.h"
#include <iostream>

#if PYTHON_ENABLED
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#if PYTHON_MULTITHREADED
struct ScriptReturnData
{
	ScriptId id = INVALID_SCRIPT_ID;
	std::string data = "";
};

ThreadSafeQueue<ScriptReturnData> returnDataQueue;
#endif

enum class ExampleEnum
{
	A = 5,
	B = 42,
	C = 100
};

void ScriptReturnValue(ExampleEnum s)
{
#if PYTHON_MULTITHREADED
	ScriptReturnData r;
	r.id = PythonInterpreter::activeScriptId;
	r.data = std::to_string((int) s);
	returnDataQueue.Push(r);
#endif
}

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(facepipe, m) {
	m.doc() = "facepipe";

	py::enum_<ExampleEnum>(m, "ExampleEnum")
		.value("A", ExampleEnum::A)
		.value("B", ExampleEnum::B)
		.value("C", ExampleEnum::C)
		.export_values();

	m.def("return", &ScriptReturnValue, "");
}

void Scripting::Initialize()
{
	python.Initialize();
}

void Scripting::Shutdown()
{
	python.Shutdown();
}

void Scripting::Tick()
{
#if PYTHON_MULTITHREADED
	ScriptExecutionResponse response;
	if (python.PopScriptResponse(response))
	{
		if (response.error == PythonScriptError::None)
			std::cout << "Script " << response.id << " executed fully" << std::endl;
		else
			std::cout << response.exception.what() << std::endl;
	}

	ScriptReturnData returnData;
	if (returnDataQueue.Pop(returnData))
	{
		if (returnData.id >= scriptReturnData.size())
			scriptReturnData.resize(returnData.id + 1);

		scriptReturnData[returnData.id] = returnData.data;

		std::cout << "Script " << returnData.id << " returned: " << returnData.data << std::endl;
	}
#else
	python.Tick();
#endif
}

bool Scripting::Execute(const std::string& code, ScriptId id)
{
	return python.Execute(code, id);
}

bool Scripting::Execute(const std::filesystem::path& filePath, ScriptId id)
{
	return python.Execute(filePath, id);
}


#else

bool Scripting::Execute(const std::string& code, ScriptId id)
{
	return false;
}

bool Scripting::Execute(const std::filesystem::path& filePath, ScriptId id)
{
	return false;
}

#endif