#include "logging.h"

#include <iostream>
#include <atomic>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include <core/threads.h>

std::atomic<bool> endLogThread = false;
std::thread logThread;

ThreadSafeQueue<std::string> logQueue;

namespace Logging
{
	void StartLoggingThread()
	{
		// Create and route stdout to custom pipes
		int pipefd[2];
		if (_pipe(pipefd, 65536, O_BINARY) == -1)
		{
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		int saved_stdout = _dup(_fileno(stdout));
		if (_dup2(pipefd[1], _fileno(stdout)) == -1)
		{
			perror("_dup2");
			exit(EXIT_FAILURE);
		}

		logThread = std::thread([pipein = pipefd[0], pipeout = pipefd[1], saved_stdout]() {
			static const int bufferSize = 1024;
			char buffer[bufferSize];

			std::string line = "";
			while (!endLogThread)
			{
				int bytesRead = _read(pipein, buffer, bufferSize);
					
				if (bytesRead > 0)
				{
					// forward to default buffer
					_write(saved_stdout, buffer, bytesRead);

					line.append(buffer, bytesRead);

					//if (line[line.size() - 1] == '\n' || bytesRead < bufferSize)
					{
						logQueue.Push(line);
						line = "";
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			// close our custom pipe and restore output to console
			_close(pipeout);
			_close(pipein);
			_dup2(saved_stdout, _fileno(stdout));
		});
	}

	void StopLoggingThread()
	{
		endLogThread = true;
		std::cout << "dummylinetotriggerthreadtostopreadingbuffer" << std::endl;
		logThread.join();
	}

	void Flush()
	{
		fflush(stdout);
		std::cout.flush();
	}

	bool GetLine(std::string& line)
	{
		return logQueue.Pop(line);
	}
}
