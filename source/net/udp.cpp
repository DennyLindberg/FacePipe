#include "udp.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "application/application.h"

// converts NetSocket to something the OS prefers
bool to_net_addr(sockaddr_in& addr, const NetSocket& info)
{
	addr.sin_family = AF_INET;
	addr.sin_port = htons(info.port);

	if (inet_pton(AF_INET, info.ip.c_str(), &addr.sin_addr) <= 0)
	{
		return false;
	}

	return true;
}

bool to_netsocket(const sockaddr_in& addr, NetSocket& info)
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

bool UDPSocket::Start()
{
	if (ossocket)
		return true;

	if (!Net::WinsockReady())
	{
		Logf(LOG_NET, "Failed to create UDP socket - Winsock is not started\n", ip, port);
		return false;
	}

	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) 
	{
		Logf(LOG_NET, "Failed to create UDP socket - socket() returned INVALID_SOCKET [{}:{}]\n", ip, port);
		return false;
	}
	ossocket = (void*) sock; // we don't own this one - void* so we don't need to include windows headers in the rest of the program

	u_long nonBlockingMode = 1;
	int result = ioctlsocket(sock, FIONBIO, &nonBlockingMode);
	if (result != NO_ERROR) 
	{
		Logf(LOG_NET, "Failed to create UDP socket - ioctlsocket() failed to set nonBlockingMode [{}:{}]\n", ip, port);
		Close();
		return false;
	}

	sockaddr_in addr;
	if (!to_net_addr(addr, *((NetSocket*)this))) 
	{
		Logf(LOG_NET, "Failed to create UDP socket - inet_pton() failed [{}:{}]\n", ip, port);
		Close();
		return false;
	}

	// Bind the socket to the address
	if (bind(sock, (struct sockaddr*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		Logf(LOG_NET, "Failed to create UDP socket - bind() failed [{}:{}]\n", ip, port);
		Close();
		return false;
	}

	Logf(LOG_NET, "Started UDP socket [{}:{}]\n", ip, port);

	return true;
}

void UDPSocket::Close()
{
	if (ossocket)
	{
		if (!Net::WinsockReady())
		{
			Logf(LOG_NET, "Failed to Close() UDP socket - Valid SOCKET but Winsock not running? \n", ip, port);
			ossocket = nullptr;
			return;
		}

		closesocket((SOCKET)ossocket);
		ossocket = nullptr;

		Logf(LOG_NET, "Closed UDP socket [{}:{}]\n", ip, port);
	}
}

bool UDPSocket::Send(const std::string& message, const NetSocket& sock)
{
	if (message.length() >= 512)
	{
		// UDP can be up to 65507 bytes but is limited by the Maximum Transmission Unit (1500 bytes or less).
		// For portability it is recommended to send < 500 bytes at a time.
		Logf(LOG_NET, "Excessive string length in UDPSocket::Send()! Message exceeded 512 bytes, the MTU might send fragmented packets which increases the risk of datagram loss.\n", ip, port);
	}

	sockaddr_in addr;
	if (to_net_addr(addr, sock) && sendto((SOCKET)ossocket, message.c_str(), (int) message.length(), 0, (SOCKADDR*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR) 
	{
		Logf(LOG_NET, "Failed to Send() message over UDP socket [{}:{}]\n", ip, port);
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
				Logf(LOG_NET, "Socket closed unexpectedly during recvfrom() [{}:{}]\n", ip, port);
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

	return bReceivedAnyDatagram;
}
