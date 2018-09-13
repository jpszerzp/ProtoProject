#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

sNetAddress::sNetAddress(const char* charArr)
{
	// split the string?
	std::string str(charArr);

	std::string str_ipv4_addr;
	std::string str_port;

	StringSplitTwo(str, ":", str_ipv4_addr, str_port);

	sockaddr saddr;
	int addrlen;
	bool got_addr = GetAddressForHost(&saddr, &addrlen, str_ipv4_addr.c_str(), str_port.c_str());

	ASSERT_OR_DIE(got_addr, "cannot get addr for host");			// exit if this fails

	bool from_success = FromSockAddr(&saddr);
	ASSERT_OR_DIE(from_success, "cannot generate ip/port");
}

sNetAddress::sNetAddress(const char* charArr, uint16_t port)
{
	std::string ipv4(charArr);
	std::string portSt = std::to_string(port);

	sockaddr saddr;
	int addrlen;
	bool got_addr = GetAddressForHost(&saddr, &addrlen, ipv4.c_str(), portSt.c_str());

	ASSERT_OR_DIE(got_addr, "cannot get addr for host");

	bool from_success = FromSockAddr(&saddr);
	ASSERT_OR_DIE(from_success, "canont generate ip/port");
}

bool sNetAddress::ToSockAddr(sockaddr* out, size_t* out_addrlen) const
{
	*out_addrlen = sizeof(sockaddr_in);

	sockaddr_in* ipv4 = (sockaddr_in*)out;
	memset(ipv4, 0, sizeof(sockaddr_in));

	ipv4->sin_addr.S_un.S_addr = ipv4_address;
	ipv4->sin_port = ::htons(port);
	ipv4->sin_family = AF_INET;
	return true;
}

bool sNetAddress::FromSockAddr(const sockaddr* sa)
{
	if (sa->sa_family != AF_INET)
	{
		return false;
	}
	
	// if IPv6 - also check AF_INET6

	const sockaddr_in* ipv4 = (const sockaddr_in*)sa;

	uint ip = ipv4->sin_addr.S_un.S_addr;
	uint16_t p = ::ntohs(ipv4->sin_port);

	ipv4_address = ip;
	port = p;

	return true;
}

std::string sNetAddress::ToString() const
{
	uint8_t* array = (uint8_t*)&ipv4_address;
	std::string str = Stringf("%u.%u.%u.%u:%u", array[0], array[1], array[2], array[3], port);
	return str;
}


bool sNetAddress::GetBindableAddr(sNetAddress* saddr, uint16_t port)
{
	// get my local ip
	char out[256];
	GetAddressString(out);
	std::string portStr = std::to_string(port);

	sockaddr sckAddr;
	int addrlen;
	bool got_addr = GetAddressForHost(&sckAddr, &addrlen, out, portStr.c_str());
	ASSERT_OR_DIE(got_addr, "cannot get addr of local host");

	bool sock_success = saddr->FromSockAddr(&sckAddr);
	ASSERT_OR_DIE(sock_success, "failed to find bindable addr for local host");

	return true;
}


bool GetAddressForHost( sockaddr* out, int* out_addrlen, const char* hostname, const char* service)
{
	// no host name - can't resolve; 
	if (StringIsNullOrEmpty(hostname))
	{
		return false; 
	}

	// there is a lot of ways to communicate with our machine
	// and many addresses associated with it - so we need to 
	// provide a hint to the API to filter down to only the addresses we care about; 
	addrinfo hints; 
	memset( &hints, 0, sizeof(hints) );  // initialized to all zero; 

	hints.ai_family = AF_INET;			// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;	// TCP socket (SOCK_DGRAM for UDP)
										//hints.ai_flags = AI_PASSIVE;		// an address we can host on;
										//hints.ai_flags |= AI_NUMERICHOST; // will speed up this function since it won't have to lookup the address; 

	addrinfo* result = nullptr; 
	int status = getaddrinfo( hostname, service, &hints, &result ); 
	if (status != 0) 
	{
		return false; 
	}

	// result now is a linked list of addresses that match our filter; 
	bool found = false;
	addrinfo* iter = result;
	while (iter != nullptr)
	{
		// you can farther filter here if you want, or return all of them and try them in order; 
		// for example, if you're using VPN, you'll get two unique addresses for yourself; 
		// if you're using AF_INET, the address is a sockaddr_in; 
		if (iter->ai_family == AF_INET)
		{
			sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr); 
			// we have an address - print it!
			memcpy(out, ipv4, sizeof(sockaddr_in));
			*out_addrlen = sizeof(sockaddr_in);
			found = true;
			break;
		}
		iter = iter->ai_next; 
	}

	// freeing up
	::freeaddrinfo( result ); 

	return found;
}

void GetAddressString()
{
	char myName[256];
	if (SOCKET_ERROR == ::gethostname(myName, 256))
	{
		return;
	}

	const char* service = "80"; // service is like "http" or "ftp", which translates to a port (80 or 21).  We'll just use port 80 for this example;

	// no host name - can't resolve; 
	if (StringIsNullOrEmpty(myName))
	{
		return; 
	}

	// there is a lot of ways to communicate with our machine
	// and many addresses associated with it - so we need to 
	// provide a hint to the API to filter down to only the addresses we care about; 
	addrinfo hints; 
	memset( &hints, 0, sizeof(hints) );  // initialized to all zero; 

	hints.ai_family = AF_INET;			// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;	// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = AI_PASSIVE;		// an address we can host on;
										//hints.ai_flags |= AI_NUMERICHOST; // will speed up this function since it won't have to lookup the address; 

	addrinfo* result = nullptr; 
	int status = getaddrinfo( myName, service, &hints, &result ); 
	if (status != 0)
	{
		LogTaggedPrintf( "net", "Failed to find addresses for [%s:%s].  Error[%s]\n", 
			myName, service, ::gai_strerror(status) );
		DebuggerPrintf("Failed to find addresses for [%s:%s].  Error[%s]\n", 
			myName, service, ::gai_strerror(status));
		return; 
	}

	// result now is a linked list of addresses that match our filter; 
	addrinfo* iter = result;
	while (iter != nullptr)
	{
		// you can farther filter here if you want, or return all of them and try them in order; 
		// for example, if you're using VPN, you'll get two unique addresses for yourself; 
		// if you're using AF_INET, the address is a sockaddr_in; 
		if (iter->ai_family == AF_INET) 
		{
			sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr); 
			// we have an address - print it!

			// if you look at the bytes - you can "see" the address, but backwards... we'll get to that
			// (port too)
			char out[256]; 
			inet_ntop( ipv4->sin_family, &(ipv4->sin_addr), out, 256 ); 
			LogTaggedPrintf( "net", "My Address: %s\n", out ); 
			DebuggerPrintf("My Address: %s\n", out);
		}
		iter = iter->ai_next; 
	}

	// freeing up
	::freeaddrinfo( result ); 
}

void GetAddressString(char* out)
{
	char myName[256];
	if (SOCKET_ERROR == ::gethostname(myName, 256))
	{
		return;
	}

	const char* service = "80"; // service is like "http" or "ftp", which translates to a port (80 or 21).  We'll just use port 80 for this example;

	// no host name - can't resolve; 
	if (StringIsNullOrEmpty(myName))
	{
		return; 
	}

	// there is a lot of ways to communicate with our machine
	// and many addresses associated with it - so we need to 
	// provide a hint to the API to filter down to only the addresses we care about; 
	addrinfo hints; 
	memset( &hints, 0, sizeof(hints) );  // initialized to all zero; 

	hints.ai_family = AF_INET;			// IPv4 Addresses
	hints.ai_socktype = SOCK_STREAM;	// TCP socket (SOCK_DGRAM for UDP)
	hints.ai_flags = AI_PASSIVE;		// an address we can host on;
										//hints.ai_flags |= AI_NUMERICHOST; // will speed up this function since it won't have to lookup the address; 

	addrinfo* result = nullptr; 
	int status = getaddrinfo( myName, service, &hints, &result ); 
	if (status != 0)
	{
		LogTaggedPrintf( "net", "Failed to find addresses for [%s:%s].  Error[%s]\n", 
			myName, service, ::gai_strerror(status) );
		DebuggerPrintf("Failed to find addresses for [%s:%s].  Error[%s]\n", 
			myName, service, ::gai_strerror(status));
		return; 
	}

	// result now is a linked list of addresses that match our filter; 
	addrinfo* iter = result;
	while (iter != nullptr)
	{
		// you can farther filter here if you want, or return all of them and try them in order; 
		// for example, if you're using VPN, you'll get two unique addresses for yourself; 
		// if you're using AF_INET, the address is a sockaddr_in; 
		if (iter->ai_family == AF_INET) 
		{
			sockaddr_in* ipv4 = (sockaddr_in*)(iter->ai_addr); 
			// we have an address - print it!

			// if you look at the bytes - you can "see" the address, but backwards... we'll get to that
			// (port too)
			inet_ntop( ipv4->sin_family, &(ipv4->sin_addr), out, 256 ); 
			LogTaggedPrintf( "net", "My Address: %s\n", out ); 
			DebuggerPrintf("My Address: %s\n", out);
		}
		iter = iter->ai_next; 
	}

	// freeing up
	::freeaddrinfo( result ); 
}

//------------------------------------------------------------------------
// Connect/Send/Recv Example
void Connect( const sNetAddress& addr, const char* msg )
{
	// 1 - create socket

	// create the socket - this allows us to setup options; 
	SOCKET sock = socket( AF_INET,   // address (socket) family (IPv4)
		SOCK_STREAM,                  // type (TCP is a stream based packet)
		IPPROTO_TCP );                // protocol, TCP

	if (sock == INVALID_SOCKET) 
	{
		ConsolePrintfUnit(Rgba::RED, "Could not create socket");
		return; 
	}

	// 2 - convert to general sockaddr_storage

	// sockaddr storage is a sockaddr struct that
	// is large enough to fit any other sockaddr struct
	// sizeof(sockaddr_storage) >= sizeof(any other sockaddr)
	sockaddr_storage saddr;
	size_t addrlen; 
	ASSERT_OR_DIE(addr.ToSockAddr( (sockaddr*) &saddr, &addrlen ), "cannot resolve address"); 
	//CONFIRM( addr.ToSockAddr( (sockaddr*) &saddr, &addrlen ) ); 
	// CONFIRM asserts in debug if it evaluates to false, and release just runs the inside code;

	// 3 - connect

	// we then try to connect.  This will block until it either succeeds
	// or fails (which possibly requires a timeout).  We will talk about making
	// this non-blocking later so it doesn't hitch the game (more important 
	// when connecting to remote hosts)
	int result = ::connect( sock, (sockaddr*)&saddr, (int)addrlen ); 
	if (result == SOCKET_ERROR)
	{
		ConsolePrintfUnit( Rgba::RED, "Connect fails\n" ); 
		::closesocket(sock);  // frees the socket resource; 
		return; 
	}
	ConsolePrintfUnit(Rgba::GREEN, "Connect succeeds");
	LogTaggedPrintf("net", "Connected.\n");

	// 4 - send

	// you send raw bytes over the network in whatever format you want; 
	char payload[256]; 
	::send( sock, msg, (int)StringGetSize(msg), 0 ); 

	// 5 - receive

	// with TCP/IP, data sent together is not guaranteed to arrive together.  
	// so make sure you check the return value.  This will return SOCKET_ERROR
	// if the host disconnected, or if we're non-blocking and no data is there. 
	size_t recvd = ::recv( sock, payload, 256 - 1U, 0 ); 

	// it may not be null terminated and I'm printing it, so just for safety. 
	payload[recvd] = NULL; 
	ConsolePrintfUnit( Rgba::GREEN, "Recieved: %s\n", payload ); 

	// 6 - close

	// cleanup
	::closesocket(sock); 
}

void TCPConnect(const sNetAddress& addr, const char* msg)
{
	// initialize socket
	//TCPSocket socket = TCPSocket(false);
	TCPSocket socket = TCPSocket();

	if (socket.Connect(addr))
	{
		socket.SetBlockMode(false);
		socket.Send(msg, StringGetSize(msg));

		char payload[256];
		size_t recvd = socket.Receive(payload, 256 - 1U);
		payload[recvd] = NULL;

		socket.Close();
	}
}

//------------------------------------------------------------------------
// Listen (Host) Example
void Host( uint16_t port )
{
	sNetAddress addr;

	/////////////////////////////////// listen ////////////////////////////////////
	// write this yourself; 
	if (!sNetAddress::GetBindableAddr( &addr, port )) 
	{ 
		ConsolePrintfUnit(Rgba::RED, "Cannot get host addr");
		return; 
	}
	
	// now we have an address, we can try to bind it; 
	// first, we create a socket like we did before; 
	SOCKET sock = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); 

	// Next, we bind it - which means we assign an address to it; 
	sockaddr_storage saddr;
	size_t addrlen; 
	addr.ToSockAddr( (sockaddr*)&saddr, &addrlen ); 

	size_t res = ::bind( sock, (sockaddr*)&saddr, (int)addrlen ); 
	if (res == SOCKET_ERROR) 
	{
		// failed to bind - if you want to know why, call WSAGetLastError()
		ConsolePrintfUnit(Rgba::RED, "Bind fail");
		::closesocket(sock); 
		return; 
	}

	// we now have a bound socket - this means we can start listening on it; 
	// This allows the socket to queue up connections - like opening a call center.
	// The number passed is NOT the max number of connections - just the max number 
	// of people who can be waiting to connect at once (think of it has a call center 
	// with N people manning the phone, but many more people who calls may be forwarded 
	// to.  Once a call is forwarded, the person answering calls can now answer a new one)
	int max_queued = 16;  // probably pick a number that is <= max number of players in your game. 
	res = ::listen( sock, max_queued ); 
	if (res == SOCKET_ERROR) 
	{
		ConsolePrintfUnit(Rgba::RED, "Listen fail");
		::closesocket(sock); 
		return; 
	}
	//////////////////////////////// end of listen //////////////////////////////////

	// now we can "accept" calls. 
	bool is_running = true; 
	while (is_running) 
	{
		// this will create a new socket
		// and fill in the address associated with that socket (who are you talking to?)
		sockaddr_storage their_addr; 
		int their_addrlen = sizeof(sockaddr_storage); 
		SOCKET their_sock = ::accept( sock, (sockaddr*)&their_addr, &their_addrlen ); 

		if (sock != INVALID_SOCKET) 
		{
			char buffer[256]; 
			// great, we have a socket - we can now send and receive on it just as before
			// with this test - we'll always listen, then send, but 
			int recvd = ::recv( their_sock, 
				buffer,           // what we read into
				256 - 1U,         // max we can read
				0U );             // flags (unused)

			if (recvd != SOCKET_ERROR)
			{
				buffer[recvd] = NULL; // just cause I'm printing it

				// me sending a response, the word PONG which is 5 long (when you include the null terminator)
				::send( their_sock, "PONG", 5, 0U ); 
			}

			// clean up after you're done; 
			::closesocket(their_sock); 
		}

		// something should tell me to stop servicing requests eventually
		//is_running = false; 
	}

	// and I'm done; 
	::closesocket(sock); 
}

void TCPHost(uint16_t port)
{
	int max_queued = 16;  // probably pick a number that is <= max number of players in your game. 
	TCPSocket host = TCPSocket(); 
	if (!host.Listen( port, max_queued ))
	{
		return; 
	} 
	host.SetBlockMode(false);

	FD_SET readfds;

	bool is_running = true;
	while (is_running) 
	{
		FD_ZERO(&readfds);
		FD_SET(host.m_handle, &readfds);

		select(0, &readfds, NULL, NULL, NULL);

		TCPSocket* them = host.Accept();
		if (them != nullptr)
		{
			them->SetBlockMode(false);

			char buffer[256]; 
			size_t received = them->Receive( buffer, 256 - 1U ); 
			if (received > 0U)
			{
				buffer[received] = NULL;
				ConsolePrintfUnit( Rgba::GREEN, "Received: %s\n", buffer ); 

				them->Send( "PONG", 5 ); 
			}

			delete them; 
			them = nullptr;
		}

		// something should tell me to stop servicing requests eventually
		//is_running = false; 
	}

	host.Close(); 
}
