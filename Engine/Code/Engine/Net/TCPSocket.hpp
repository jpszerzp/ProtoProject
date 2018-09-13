#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Net/NetAddress.hpp"

class TCPSocket
{
public:
	TCPSocket();
	TCPSocket(SOCKET handle, sNetAddress addr);
	~TCPSocket(); 

	// client
	bool Connect( const sNetAddress& addr ); 

	// host
	bool Listen( uint16_t port, uint max_queued); 
	TCPSocket* Accept(); 

	// when finished
	void Close(); 

	// how much sent
	size_t Send( const void* data, const size_t data_byte_size ); 
	// templated overload
	template<typename T>
	size_t Send(const T value);
	// returns how much received
	size_t Receive( void* buffer, const size_t max_byte_size ); 

	// utilities
	bool IsClosed() const; 
	void CheckDisconnect();
	void SetBlockMode(bool blocking);

public:
	SOCKET m_handle; 

	bool m_listens;

	// if you're a listening, the address is YOUR address
	// if you are connecting (or socket is from an accept)
	// this address is THEIR address;  (do not listen AND connect on the same socket)
	sNetAddress m_address; 
};

template<typename T>
size_t TCPSocket::Send(const T value)
{
	if (m_handle == INVALID_SOCKET)
		return 0;

	size_t byteNum = sizeof(T);

	uint8_t* valuePtr = (uint8_t*)(&value);

	int bytes_sent = ::send(m_handle, (const char*)valuePtr, (int)byteNum, 0);

	if (bytes_sent == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			ConsolePrintfUnit(Rgba::YELLOW, 
				"Socket is non-blocking, but SEND will block", err);
			return 0;
		}
		ConsolePrintfUnit(Rgba::RED, "Send fails due to %i", err);
		Close();
		return 0;
	}

	return bytes_sent;
}

bool HasFatalError(int errCode);