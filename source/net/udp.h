#pragma once

#include <functional>
#include "netsocket.h"
#include "facepipe.h"

struct UDPDatagram
{
	NetSocket source;
	std::vector<char> message;
	FacePipe::MetaData metaData; // empty until parsed
};

class UDPSocket : public NetSocket
{
protected:
	void* ossocket = nullptr;

public:
	static std::function<void(const char*)> Logger;
	double bReceivedDataLastCall = false; // UI status hack

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

	bool IsConnected() const { return ossocket != nullptr; }

	std::string ToString() const;
};