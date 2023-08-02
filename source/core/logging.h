#pragma once

#include <string>

namespace Logging
{
	void StartLoggingThread();
	void StopLoggingThread();
	bool GetLine(std::string& line);
}
