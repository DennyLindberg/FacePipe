#pragma once

#include <functional>
#include "netsocket.h"
#include "facepipe.h"

struct UDPDatagram
{
	NetAddressIP4 source;
	std::vector<char> message;
	FacePipe::MessageInfo metaData; // empty until parsed
};

class UDPSocket : public NetAddressIP4
{
protected:
	void* ossocket = nullptr;

public:
	static std::function<void(const char*)> Logger;
	double bReceivedDataLastCall = false; // UI status hack

	UDPSocket(const char* socketIP = Net::LocalHost, int socketPort = 0)
		: NetAddressIP4(socketIP, socketPort)
	{
	}

	UDPSocket(int socketPort)
		: NetAddressIP4(socketPort)
	{
	}

	~UDPSocket() 
	{
		Close();
	}

	void Set(const char* socketIP, int socketPort);
	bool Start();
	void Close();

	bool Send(const std::string& message, const NetAddressIP4& target);
	bool Send(const UDPDatagram& datagram, const NetAddressIP4& target);
	bool Receive(std::vector<UDPDatagram>& datagrams);

	bool IsConnected() const { return ossocket != nullptr; }

	std::string ToString() const;
};