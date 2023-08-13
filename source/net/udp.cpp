#include "udp.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <format>

std::function<void(const char*)> UDPSocket::Logger = [](const char*) -> void {};

#define UDPLog(str, ...) UDPSocket::Logger(std::format(str, __VA_ARGS__).c_str())

// converts NetSocket to something the OS prefers
bool to_net_addr(sockaddr_in& addr, const NetAddressIP4& info)
{
	addr.sin_family = AF_INET;
	addr.sin_port = htons(info.port);

	if (inet_pton(AF_INET, info.ip.c_str(), &addr.sin_addr) <= 0)
	{
		return false;
	}

	return true;
}

bool to_netsocket(const sockaddr_in& addr, NetAddressIP4& info)
{
	char from_ip[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(addr.sin_addr), from_ip, INET_ADDRSTRLEN))
	{
		info.ip = from_ip;
		info.port = addr.sin_port;
		return true;
	}
	else
	{
		return false;
	}
}

void UDPSocket::Set(const char* socketIP, int socketPort)
{
	if (ossocket)
	{
		Close();
	}
	
	ip = socketIP;
	port = socketPort;
}

bool UDPSocket::Start()
{
	if (ossocket)
		return true;

	if (!Net::WinsockReady())
	{
		UDPLog("Failed to create UDP socket - Winsock is not started\n", ip, port);
		return false;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) 
	{
		UDPLog("Failed to create UDP socket - socket() returned INVALID_SOCKET [{}:{}]\n", ip, port);
		return false;
	}
	ossocket = (void*) sock; // we don't own this one - void* so we don't need to include windows headers in the rest of the program

	u_long nonBlockingMode = 1;
	int result = ioctlsocket(sock, FIONBIO, &nonBlockingMode);
	if (result != NO_ERROR) 
	{
		UDPLog("Failed to create UDP socket - ioctlsocket() failed to set nonBlockingMode [{}:{}]\n", ip, port);
		Close();
		return false;
	}

	sockaddr_in addr;
	if (!to_net_addr(addr, *((NetAddressIP4*)this))) 
	{
		UDPLog("Failed to create UDP socket - inet_pton() failed [{}:{}]\n", ip, port);
		Close();
		return false;
	}

	// Bind the socket to the address
	if (bind(sock, (struct sockaddr*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		UDPLog("Failed to create UDP socket - bind() failed [{}:{}]\n", ip, port);
		Close();
		return false;
	}

	UDPLog("Started UDP socket [{}]\n", ToString());

	return true;
}

void UDPSocket::Close()
{
	if (ossocket)
	{
		if (!Net::WinsockReady())
		{
			UDPLog("Failed to Close() UDP socket - Valid SOCKET but Winsock not running? \n", ip, port);
			ossocket = nullptr;
			return;
		}

		closesocket((SOCKET)ossocket);
		ossocket = nullptr;

		UDPLog("Closed UDP socket [{}:{}]\n", ip, port);
	}
}

bool UDPSocket::Send(const std::string& message, const NetAddressIP4& target)
{
	//if (message.length() >= 512)
	//{
	//	// UDP can be up to 65507 bytes but is limited by the Maximum Transmission Unit (1500 bytes or less).
	//	// For portability it is recommended to send < 500 bytes at a time.
	//	UDPLog("Excessive string length in UDPSocket::Send()! Message exceeded 512 bytes, the MTU might send fragmented packets which increases the risk of datagram loss.\n", ip, port);
	//}

	sockaddr_in sock_addr;
	if (to_net_addr(sock_addr, target) && sendto((SOCKET)ossocket, message.c_str(), (int) message.length(), 0, (SOCKADDR*)&sock_addr, sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		UDPLog("Failed to Send() message over UDP socket [{}:{}]\n", ip, port);
		return false;
	}

	return true;
}

bool UDPSocket::Send(const UDPDatagram& datagram, const NetAddressIP4& target)
{
	sockaddr_in sock_addr;
	if (to_net_addr(sock_addr, target) && sendto((SOCKET)ossocket, datagram.message.data(), (int)datagram.message.size(), 0, (SOCKADDR*)&sock_addr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		UDPLog("Failed to Send() message over UDP socket [{}:{}]\n", ip, port);
		return false;
	}

	return true;
}

bool is_socket_valid(SOCKET sock) 
{
	char optval;
	int optlen = sizeof(optval);
	if (getsockopt(sock, SOL_SOCKET, SO_TYPE, &optval, &optlen) == SOCKET_ERROR) 
	{
		int error_code = WSAGetLastError();
		if (error_code == WSAENOTSOCK || error_code == WSAEINVAL) 
		{
			return false;
		}
	}

	return true;
}

bool UDPSocket::Receive(std::vector<UDPDatagram>& datagrams)
{
	static const int bufferLength = 65535; // 16 bit max value, max theoretical size for a UDP datagram is 65507
	static char buffer[bufferLength]; // Buffer to hold received data

	bool bReceivedAnyDatagram = false;
	int bytes_received = 0;
	do 
	{
		sockaddr_in sender_addr{};
		int sender_len = sizeof(sender_addr);
		int bytes_received = recvfrom((SOCKET)ossocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender_addr, &sender_len);

		if (bytes_received == SOCKET_ERROR) 
		{	
			int error_code = WSAGetLastError();
			if (error_code == WSAEWOULDBLOCK)
			{
				break; // no more incoming data
			}
			else if (error_code == WSAENETDOWN || error_code == WSAESHUTDOWN || error_code == WSAENOTCONN || !is_socket_valid((SOCKET)ossocket))
			{
				Close();
				UDPLog("Socket closed unexpectedly during recvfrom() [{}:{}]\n", ip, port);
				break;
			}
			else
			{
				// Other errors should be harmless
				break;
			}
		}
		else if (bytes_received > 0)
		{
			bReceivedAnyDatagram = true;
			datagrams.push_back(UDPDatagram());
			datagrams.back().message.assign(buffer, buffer + bytes_received);
			to_netsocket(sender_addr, datagrams.back().source);
		}
	} while (bytes_received > 0);
	
	bReceivedDataLastCall = bReceivedAnyDatagram;
	return bReceivedAnyDatagram;
}

std::string UDPSocket::ToString() const
{
	sockaddr_in localAddress;
	int addrSize = sizeof(localAddress);

	if (getsockname((SOCKET)ossocket, (struct sockaddr*)&localAddress, &addrSize) != SOCKET_ERROR) 
	{
		// get info from socket if it is bound
		char ipAddress[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &localAddress.sin_addr, ipAddress, INET_ADDRSTRLEN);
		return std::format("{}:{}", ipAddress,  ntohs(localAddress.sin_port));
	}
	else
	{
		// just use the user-requested values
		return std::format("{}:{}", ip, port);
	}
}
