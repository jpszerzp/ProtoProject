#include "Engine/Net/Socket.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

Socket::Socket()
{
	m_handle = INVALID_SOCKET;
}

Socket::~Socket()
{

}

void Socket::SetBlocking(bool blocking)
{
	if (!IsClosed())
	{
		u_long nonBlocking = blocking ? 0 : 1;
		::ioctlsocket(m_handle, FIONBIO, &nonBlocking);
	}
}

void Socket::Close()
{
	if (m_handle != INVALID_SOCKET)
	{
		::closesocket(m_handle);
		m_handle = INVALID_SOCKET;
	}
}

bool Socket::IsClosed()
{
	return m_handle == INVALID_SOCKET;
}

const sNetAddress& Socket::GetAddr() const
{
	return m_addr;
}

bool UDPSocket::Bind(const sNetAddress& saddr, uint16_t port_range /*= 0U */)
{
	SOCKET my_sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	ASSERT_OR_DIE(my_sock != INVALID_SOCKET, "UDP socket is invalid");

	sockaddr_storage sock_addr;
	size_t sock_addr_len;
	saddr.ToSockAddr((sockaddr*)&sock_addr, &sock_addr_len);

	int result = ::bind(my_sock, (sockaddr*)&sock_addr, (int)sock_addr_len);
	if (result == 0)
	{
		m_handle = (SOCKET)my_sock;
		m_addr = saddr;
		return true;
	}

	return false;
}

size_t UDPSocket::SendTo(const sNetAddress& saddr, const void* buffer, const size_t byte_count)
{
	if (IsClosed())
		return 0;

	sockaddr_storage addr_storage;
	size_t addr_len;
	saddr.ToSockAddr((sockaddr*)&addr_storage, &addr_len);

	SOCKET sock = (SOCKET)m_handle;
	int sent = ::sendto(sock, (const char*)buffer, (int)byte_count, 0, (sockaddr*)&addr_storage, addr_len);

	if (sent > 0)
	{
		ASSERT_OR_DIE(sent == byte_count, "Send size not match");
		return (size_t)sent;
	}
	else
	{
		int err = WSAGetLastError();
		if (HasFatalError(err))
			Close();
		return 0;
	}
}

size_t UDPSocket::ReceiveFrom(sNetAddress* out_addr, void* out_buffer, const size_t max_buffer_size)
{
	if (IsClosed())
		return 0;

	sockaddr_storage fromaddr;
	int addr_len = sizeof(sockaddr_storage);
	SOCKET sock = (SOCKET)m_handle;

	int recvd = ::recvfrom(sock, 
		(char*)out_buffer,
		(int)max_buffer_size,
		0,
		(sockaddr*)&fromaddr,
		&addr_len);

	if (recvd > 0)
	{
		out_addr->FromSockAddr((sockaddr*)&fromaddr);
		return recvd;
	}
	else
	{
		int err = WSAGetLastError();
		if (HasFatalError(err))
			Close();
		return 0;
	}
}

bool IsFatalSocketError(int errCode)
{
	return false;
}

bool HasFatalError(int errCode)
{
	if ((errCode == WSAEWOULDBLOCK) || (errCode == WSAEMSGSIZE) || (errCode == WSAECONNRESET))
		return false;
	return true;
}

UDPTest* UDPTest::m_instance = nullptr;

UDPTest* UDPTest::GetInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new UDPTest();
		m_instance->m_udp_socket = new UDPSocket();
	}
	return m_instance;
}

void UDPTest::DestroyInstance()
{
	delete m_instance;
	m_instance = nullptr;
}

bool UDPTest::Start()
{
	sNetAddress addr;
	uint count = sNetAddress::GetBindableAddr(&addr, GAME_PORT);
	if (count == 0U)
	{
		ConsolePrintfUnit(Rgba::RED, "UDP: No addr");
		return false;
	}

	if (!m_udp_socket->Bind(addr, 10))
	{
		ConsolePrintfUnit(Rgba::RED, "UDP: Failed to bind");
		return false;
	}
	else
	{
		m_udp_socket->SetBlocking(false);
		ConsolePrintfUnit(Rgba::GREEN, "UDP: Socket bound: %s", m_udp_socket->GetAddr().ToString().c_str());
		return true;
	}
}

void UDPTest::Stop()
{
	m_udp_socket->Close();
}

void UDPTest::SendTo(const sNetAddress& saddr, const void* buffer, uint byte_count)
{
	m_udp_socket->SendTo(saddr, buffer, byte_count);
}

void UDPTest::Update()
{
	char buffer[PACKET_MTU];

	sNetAddress from_addr;
	size_t read = m_udp_socket->ReceiveFrom(&from_addr, buffer, PACKET_MTU);

	if (read > 0U)
	{
		uint max_bytes = min(read, 128);

		uint string_size = max_bytes * 2U + 3U;
		char* new_buff = new char[string_size];
		sprintf_s(new_buff, 3U, "0x");
		char* iter = new_buff;
		iter += 2U;

		for (uint i = 0; i < read; ++i)
		{
			sprintf_s(iter, 3U, "%02X", buffer[i]);
			iter += 2U;
		}
		*iter = NULL;

		ConsolePrintfUnit(Rgba::GREEN, "Received from %s;\n%s", from_addr.ToString().c_str(), new_buff);
		delete[] buffer;
	}
}
