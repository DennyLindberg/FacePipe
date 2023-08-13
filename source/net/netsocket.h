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

class NetAddressIP4
{
public:
	NetAddressIP4(const char* AddressIP = Net::LocalHost, int AddressPort = 0)
		: ip(AddressIP), port(AddressPort)
	{}

	NetAddressIP4(int AddressPort)
		: ip(Net::LocalHost), port(AddressPort)
	{}

	std::string ip = Net::LocalHost;
	int port = 0;
	int id = 0; // set by internals
};
