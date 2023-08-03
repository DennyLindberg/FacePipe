#pragma once

#include <string>

namespace Net
{
	static const char* LocalHost = "127.0.0.1";
	static const char* LocalAll = "0.0.0.0";
	bool WinsockReady();
	int StartWinsock();
	void StopWinsock();
}

class NetSocket
{
public:
	NetSocket(const char* socketIP = Net::LocalHost, int socketPort = 0)
		: ip(socketIP), port(socketPort)
	{}

	NetSocket(int socketPort)
		: ip(Net::LocalHost), port(socketPort)
	{}

	std::string ip = Net::LocalHost;
	int port = 0;
	int id = 0; // set by internals
};
