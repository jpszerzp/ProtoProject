#pragma once

#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/BytePacker.hpp"

#define GAME_PORT 10084
#define ETHERNET_MTU 1500
#define PACKET_MTU (ETHERNET_MTU - 40 - 8)

enum eSocketOptionBit : uint 
{
	SOCKET_OPTION_BLOCKING = BIT_FLAG(0),

	// some other options that may be useful to support
	// SOCKET_OPTION_BROADCAST - socket can broadcast messages (if supported by network)
	// SOCKET_OPTION_LINGER - wait on close to make sure all data is sent
	// SOCKET_OPTION_NO_DELAY - disable nagle's algorithm
};
typedef uint eSocketOptions; 

class Socket 
{
public:
	Socket();
	virtual ~Socket();

	void SetBlocking(bool blocking);

	void Close();
	bool IsClosed();
	
	const sNetAddress& GetAddr() const;

public:
	sNetAddress m_addr;
	SOCKET m_handle;
	eSocketOptions m_options;
};

bool IsFatalSocketError(int errCode);
bool HasFatalError(int errCode);

class UDPSocket : public Socket
{
public:
	bool Bind(const sNetAddress& saddr, uint16_t port_range = 0U );
	size_t SendTo(const sNetAddress& saddr, const void* buffer, const size_t byte_count);
	size_t ReceiveFrom(sNetAddress* out_addr, void* buffer, const size_t max_read_size);
};

class UDPTest 
{
public:
	UDPSocket* m_udp_socket = nullptr;
	static UDPTest* m_instance;
public:
	static UDPTest* GetInstance();
	static void DestroyInstance();

	bool Start();
	void Stop();
	void SendTo(const sNetAddress& saddr, const void* buffer, uint byte_count);
	void Update();
};