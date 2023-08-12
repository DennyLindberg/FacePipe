#pragma once

#include <string>
#include <queue>
#include "netsocket.h"

struct UDPDatagram
{
	NetSocket source;
	std::vector<char> message;
};

class UDPSocket : public NetSocket
{
protected:
	void* ossocket = nullptr;

public:
	bool bReceivedDataLastCall = false; // UI status hack

	UDPSocket(const char* socketIP = Net::LocalHost, int socketPort = 0)
		: NetSocket(socketIP, socketPort)
	{
	}

	UDPSocket(int socketPort)
		: NetSocket(socketPort)
	{
	}

	~UDPSocket() 
	{
		Close();
	}

	void Set(const char* socketIP, int socketPort);
	bool Start();
	void Close();

	bool Send(const std::string& message, const NetSocket& sock);
	bool Receive(std::vector<UDPDatagram>& datagrams);
};