#pragma once

#include "python/python.h"

class Scripting
{
public:
	Scripting() {}
	~Scripting() {}

	PythonInterpreter python;

	void Initialize();
	void Shutdown();
	void Tick();

	std::vector<std::string> scriptReturnData;
};
