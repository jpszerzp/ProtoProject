#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

TCPSocket::TCPSocket(SOCKET handle, sNetAddress addr)
{
	m_handle = handle;
	m_address = addr;
	m_listens = false;
}

TCPSocket::TCPSocket()
{
	m_handle = INVALID_SOCKET;
	m_listens = false;
}

TCPSocket::~TCPSocket()
{

}

bool TCPSocket::Listen(uint16_t port, uint max_queued)
{
	// socket is in use
	if (!IsClosed())
		return false;

	sNetAddress addr;
	if (!sNetAddress::GetBindableAddr( &addr, port )) 
	{ 
		ConsolePrintfUnit(Rgba::RED, "Cannot get host addr");
		return false; 
	}

	SOCKET sock = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ); 

	if (sock == INVALID_SOCKET)
		return false;

	sockaddr_storage saddr;
	size_t addrlen; 
	addr.ToSockAddr( (sockaddr*)&saddr, &addrlen ); 

	size_t res = ::bind( sock, (sockaddr*)&saddr, (int)addrlen ); 
	if (res == SOCKET_ERROR) 
	{
		int err = WSAGetLastError();
		ConsolePrintfUnit(Rgba::RED, "Bind fails due to error %i", err);
		Close();
		return false; 
	}

	res = ::listen( sock, max_queued ); 
	if (res == SOCKET_ERROR) 
	{
		int err = WSAGetLastError();
		ConsolePrintfUnit(Rgba::RED, "Listen fails due to error %i", err);
		Close();
		return false; 
	}

	m_handle = sock;			// MY addr
	m_address = addr;		
	m_listens = true;			// this is a listen socket

	return true;
}

TCPSocket* TCPSocket::Accept()
{
	if (!m_listens)
		return nullptr;

	sockaddr_storage their_addr;
	int their_addrlen = sizeof(sockaddr_storage);
	SOCKET their_socket = ::accept(m_handle, (sockaddr*)&their_addr, &their_addrlen);

	if (their_socket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			//ConsolePrintfUnit(Rgba::YELLOW, "Socket is non-blocking, but ACCEPT will block");
			return nullptr;
		}
		ConsolePrintfUnit(Rgba::RED, "Accept fails");
		Close();
		return nullptr;
	}
	ConsolePrintfUnit(Rgba::GREEN, "Accept succeeds");

	sNetAddress saddr;
	if (!saddr.FromSockAddr((sockaddr*)&their_addr))
	{
		// close accept socket
		::closesocket(their_socket);
		return nullptr;
	}

	// THEIR addr
	TCPSocket* their_tcp_socket = new TCPSocket(their_socket, saddr);
	return their_tcp_socket;
}

bool TCPSocket::Connect(const sNetAddress& addr)
{
	// this socket is in use
	if (m_handle != INVALID_SOCKET)
	{
		return false;
	}
	SOCKET sock = socket( AF_INET,   
		SOCK_STREAM,                 
		IPPROTO_TCP );    
	if (sock == INVALID_SOCKET)
		return false;

	sockaddr_storage saddr;
	size_t addrlen;
	ASSERT_RECOVERABLE(addr.ToSockAddr( (sockaddr*) &saddr, &addrlen ), "cannot resolve address"); 

	int result = ::connect( sock, (sockaddr*)&saddr, (int)addrlen ); 
	if (result == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err == WSAEWOULDBLOCK)
		{
			ConsolePrintfUnit(Rgba::YELLOW, 
				"Socket is non-blocking, but CONNECT will block");
			return false;
		}
		ConsolePrintfUnit(Rgba::RED, "Connect fails");
		Close();
		return false;
	}

	ConsolePrintfUnit(Rgba::GREEN, "Connect succeeds");

	// connected
	m_handle = sock;
	m_address = addr;	// THEIR addr

	return true;
}

void TCPSocket::Close()
{
	if (m_handle != INVALID_SOCKET)
	{
		::closesocket(m_handle);
		m_handle = INVALID_SOCKET;
		ConsolePrintfUnit(Rgba::GREEN, "Socket closed");
	}
}


size_t TCPSocket::Send(const void* data, const size_t data_byte_size)
{
	if (m_handle == INVALID_SOCKET || data_byte_size == 0)
		return 0;

	int bytes_sent = ::send(m_handle, (const char*)data, (int)data_byte_size, 0);

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

	//ConsolePrintfUnit(Rgba::GREEN, "Send succeeds");
	return bytes_sent;
}

size_t TCPSocket::Receive(void* buffer, const size_t max_byte_size)
{
	if (m_handle == INVALID_SOCKET || max_byte_size == 0)
		return 0;

	int recvd = ::recv( m_handle, (char*)buffer, (int)max_byte_size, 0U ); 

	if (recvd <= 0)
	{
		if (recvd == SOCKET_ERROR)		// -1
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
			{
				//ConsolePrintfUnit(Rgba::YELLOW, "Socket is non-blocking, but RECEIVE will block");
				//ConsolePrintfUnit(Rgba::YELLOW, "Recieved: %s\n", buffer); 
				return 0;		// ignore, do not close
			}
			else
			{
				ConsolePrintfUnit(Rgba::RED, "Receive fails fatal %i", err); 
				Close();		// otherwise fatal error, close
			}
		}
		else
		{
			// 0, gracefully closed
			ConsolePrintfUnit(Rgba::YELLOW, "Check gracefully closed"); 
			CheckDisconnect();		// make sure gracefully closed
		}
		return 0;
	}
	else
	{
		ConsolePrintfUnit(Rgba::GREEN, "Receive succeeds");
		ConsolePrintfUnit(Rgba::GREEN, "Received: %s\n", buffer); 
		return recvd;
	}
}

bool TCPSocket::IsClosed() const
{
	return m_handle == INVALID_SOCKET;
}


void TCPSocket::CheckDisconnect()
{
	WSAPOLLFD fd;
	fd.fd = m_handle;
	fd.events = POLLRDNORM;

	if (SOCKET_ERROR == ::WSAPoll(&fd, 1, 0))
	{
		// Something went wrong – socket no longer valid; 
		Close();
	}
	if ((fd.revents & POLLHUP) != 0)
	{
		// Socket was [H]ung-[UP]
		Close();
	}
}

void TCPSocket::SetBlockMode(bool blocking)
{
	if (!IsClosed())
	{
		u_long nonBlocking = blocking ? 0 : 1;
		::ioctlsocket(m_handle, FIONBIO, &nonBlocking);
	}
}