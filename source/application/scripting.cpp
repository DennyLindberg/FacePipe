#include "scripting.h"

#include "core/threads.h"
#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>


struct ScriptReturnData
{
	ScriptId id = INVALID_SCRIPT_ID;
	std::string data = "";
};

ThreadSafeQueue<ScriptReturnData> returnDataQueue;

enum class Sentiment
{
	Angry = 0,
	Happy,
	Confused
};

void ScriptReturnValue(Sentiment s)
{
	ScriptReturnData r;
	r.id = PythonInterpreter::activeScriptId;
	r.data = std::to_string((int) s);
	returnDataQueue.Push(r);
}

namespace py = pybind11;

PYBIND11_EMBEDDED_MODULE(facepipe, m) {
	m.doc() = "facepipe";

	py::enum_<Sentiment>(m, "Sentiment")
		.value("Angry", Sentiment::Angry)
		.value("Happy", Sentiment::Happy)
		.value("Confused", Sentiment::Confused)
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
}
