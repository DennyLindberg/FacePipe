#include "netsocket.h"

#include <winsock2.h>
#include <ws2tcpip.h>

bool bIsStarted = false;
WSADATA wsaData = {};

bool Net::WinsockReady()
{
	return bIsStarted;
}

int Net::StartWinsock()
{
	if (!bIsStarted)
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			return -1;
		}

		bIsStarted = true;
	}

	return 0;
}

void Net::StopWinsock()
{
	if (bIsStarted)
	{
		bIsStarted = false;
		WSACleanup();
	}
}
