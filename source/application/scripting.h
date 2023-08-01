#pragma once

#define USE_PYTHON_THREADED false

#if USE_PYTHON_THREADED
#include "python/python_threaded.h"
#else
#include "python/python.h"
#endif


class Scripting
{
public:
	Scripting() {}
	~Scripting() {}

#if USE_PYTHON_THREADED
	PythonInterpreterThreaded python;
#else
	PythonInterpreter python;
#endif

	void Initialize();
	void Shutdown();
	void Tick();

	std::vector<std::string> scriptReturnData;
};
