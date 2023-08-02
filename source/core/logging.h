#pragma once

#include <string>

namespace Logging
{
	void StartLoggingThread();
	void StopLoggingThread();
	void Flush();
	bool GetLine(std::string& line);
}
