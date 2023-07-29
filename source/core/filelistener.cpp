#include "filelistener.h"
#include <Windows.h>
#include "application/application.h"

void ListenToFileChange(FileListener* Listener, std::filesystem::path folder, std::vector<OnFileChangeCallback>* fileCallbacks, std::deque<std::atomic_bool>* fileModifiedStates)
{
	/*
		Setup the listener
	*/
	HANDLE hDir = ::CreateFile(
		folder.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, // overlapped is needed for async operation
		NULL
	);

	if (hDir == INVALID_HANDLE_VALUE)
	{
		printf("\nFile listener failed - Invalid handle (bad path?): %ls\n", folder.c_str());
		return;
	}

	/*
		Start listener loop
	*/
	BYTE buffer[4096];
	DWORD dwBytesReturned = 0;
	DWORD error = 0;

	OVERLAPPED ovl = { 0 };
	ovl.hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL == ovl.hEvent)
	{
		printf("\nFile listener failed...");
		return;
	}

	while (Listener->IsRunning())
	{
		::ResetEvent(ovl.hEvent);
		error = ::ReadDirectoryChangesW(
			hDir,
			buffer, sizeof(buffer),
			FALSE,
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			&dwBytesReturned, &ovl, NULL);

		DWORD dw;
		DWORD result = ::WaitForSingleObject(ovl.hEvent, 500);

		if (result == WAIT_TIMEOUT || result != WAIT_OBJECT_0 || !::GetOverlappedResult(hDir, &ovl, &dw, FALSE))
		{
			continue;
		}

		/*
			Process changes
		*/
		BYTE* p = buffer;
		std::vector<std::wstring> detectedFilenames;
		for (;;)
		{
			FILE_NOTIFY_INFORMATION* info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(p);

			int stringLength = info->FileNameLength / sizeof(WCHAR);
			std::wstring filename = std::wstring((WCHAR*)&info->FileName, stringLength);

			for (int i = 0; i < (*fileCallbacks).size(); ++i)
			{
				if (filename == (*fileCallbacks)[i].fileName)
				{
					(*fileCallbacks)[i].lastModifiedTimestamp = App::clock.time;
					(*fileModifiedStates)[i] = true;
					// don't break, we can have multiple listeners on the same file
				}
			}

			if (!info->NextEntryOffset) break;
			p += info->NextEntryOffset;
		}
	}
}

FileListener::~FileListener()
{
	Shutdown();
}

void FileListener::Bind(std::wstring filename, FileCallbackSignature callback)
{
	callbacks.emplace_back(OnFileChangeCallback{
		filename,
		0.0, 0.0,
		callback
		});
	fileModifiedStates->emplace_back(false);
}

void FileListener::Initialize(std::filesystem::path listenToFolder)
{
	fileModifiedStates = new std::deque<std::atomic_bool>{};

	rootFolder = listenToFolder;
	listenerThread = std::thread(ListenToFileChange, this, rootFolder, &callbacks, fileModifiedStates);
}

void FileListener::Shutdown()
{
	if (IsRunning())
	{
		stopThread = true;
		if (listenerThread.joinable())
		{
			listenerThread.join();
		}
		delete fileModifiedStates;
		fileModifiedStates = nullptr;
	}
}

void FileListener::ProcessCallbacksOnMainThread()
{
	auto& modified = *fileModifiedStates;
	double currentTime = App::clock.time;

	for (int i = 0; i < callbacks.size(); ++i)
	{
		if (modified[i] && callbacks[i].callback)
		{
			// FULHACK: Add delay to read, sometimes the notification is too fast and the
			// file is actually not ready to be read.
			double timeSinceModification = currentTime - callbacks[i].lastModifiedTimestamp;
			if (timeSinceModification < 0.2)
			{
				continue;
			}

			modified[i] = false;

			// Avoid double notification spam
			double timeSinceLastCallback = currentTime - callbacks[i].lastCallbackTimestamp;
			if (timeSinceLastCallback > 0.2)
			{
				callbacks[i].callback(rootFolder / callbacks[i].fileName);
				callbacks[i].lastCallbackTimestamp = currentTime;
			}
		}
	}
}