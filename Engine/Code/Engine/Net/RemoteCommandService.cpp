#include "Engine/Net/RemoteCommandService.hpp"
#include "Engine/Net/BytePacker.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Console/Command.hpp"
#include "Engine/Math/MathUtils.hpp"

RCS* RCS::m_rcsInstance = nullptr;

RCS::RCS()
{
	
}

RCS::~RCS()
{
	//delete m_hostSocket;
	//m_hostSocket = nullptr;
}

void RCS::Update(float deltaTime)
{
	UpdateState(deltaTime);
	UpdateText();
}

void RCS::UpdateState(float deltaTime)
{
	switch (m_rcsState)
	{
	case RCS_INITIAL:
	{
		ResetDelay();
		char addr_ch[256];
		GetAddressString(addr_ch);
		sNetAddress saddr = sNetAddress(addr_ch, m_port);

		TCPSocket* join_local = new TCPSocket();

		bool local_connected = join_local->Connect(saddr);
		join_local->SetBlockMode(false);

		if (local_connected)
		{
			// client connect success, update client info 
			// i am client, so for connection list i add the socket to host
			m_connections.push_back(join_local);			// by default, host is the first, hence indexed 0
			join_local->SetBlockMode(false);

			ConsolePrintfUnit(Rgba::GREEN, ("Client joined: " + saddr.ToString()).c_str());
			m_rcsState = RCS_CLIENT;			// transition to client state
		}
		else
		{
			// join local failed
			std::string addr_str(addr_ch);
			ConsolePrintfUnit(Rgba::RED, ("Cannot connect to: " + addr_str).c_str());
			ConsolePrintfUnit(Rgba::WHITE, "Hosting");
			delete join_local;

			// start hosting instead
			m_listenSocket = new TCPSocket();

			int max_queued = 16;
			bool host_listened = m_listenSocket->Listen(m_port, max_queued);
			if (!host_listened)
			{
				ConsolePrintfUnit(Rgba::RED, "Host listen fails");

				delete m_listenSocket;
				m_listenSocket = nullptr;

				// go to delay
				m_rcsState = RCS_DELAY;
			}
			else
			{
				// hosting
				ConsolePrintfUnit(Rgba::GREEN, "Host listen succeeds");		
				m_listenSocket->SetBlockMode(false);
				m_rcsState = RCS_HOST;			// transition to host state
			}
		}
	}
		break;
	case RCS_CLIENT:
	{
		// process connections on host
		TCPSocket* to_host = m_connections[0];
		ReceiveAndRunMsg(to_host);

		// clean up disconnects
		if (to_host->IsClosed())
		{
			delete to_host;
			to_host = nullptr;

			m_connections.clear();

			// go back to initial state
			ConsolePrintfUnit(Rgba::RED, ("Client disconnected, back to INITIAL"));
			m_rcsState = RCS_INITIAL;
		}
	}
		break;
	case RCS_HOST:
	{
		TCPSocket* connection = m_listenSocket->Accept();

		// new connection set up
		if (connection != nullptr)
		{
			connection->SetBlockMode(false);
			m_connections.push_back(connection);
		}

		// process echo
		for each (TCPSocket* c in m_connections)
		{
			ReceiveAndRunMsg(c);
		}

		// cleanup disconnects
		for (int idx = (int)(m_connections.size() - 1U); idx >= 0; --idx)
		{
			TCPSocket* c = m_connections[idx];

			if (c->IsClosed())
			{
				delete c;

				m_connections[idx] = m_connections[m_connections.size() - 1U];
				m_connections.pop_back();
			}
		}
	}
		break;
	case RCS_DELAY:
	{
		while (m_delay > 0.f)
		{
			// delaying
			m_delay -= deltaTime;
		}

		// delay elapsed, go to INITIAL
		m_rcsState = RCS_INITIAL;
	}
		break;
	case NUM_RCS_STATE:
		break;
	default:
		break;
	}
}

void RCS::UpdateText()
{
	switch (m_rcsState)
	{
	case RCS_INITIAL:
		break;
	case RCS_CLIENT:
	{
		///////////////////////////////////////////// ADDR INFO MESH /////////////////////////////////////////////
		// i am client, addr text mesh shows this
		TCPSocket* to_host = m_connections[0];
		std::string join_info = "[client]join addr: " + to_host->m_address.ToString();
		if (m_addrMesh != nullptr)
		{
			delete m_addrMesh;
			m_addrMesh = nullptr;
		}
		Renderer* renderer = Renderer::GetInstance();
		m_addrMMin = m_titleMMin - Vector2(0.f, m_addrMHeight);
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		m_addrMesh = Mesh::CreateTextImmediate(Rgba::WHITE, m_addrMMin, font, 
			m_addrMHeight, m_addrMRatio, join_info, VERT_PCU);
		///////////////////////////////////////////// CONNECTION NUMBER MESH /////////////////////////////////////////////
		std::string connect_count_str = std::to_string(m_connections.size());
		std::string max_connect_str = std::to_string(MAX_CONNECTIONS);
		std::string client_str = "client(s): " + connect_count_str + "/" + max_connect_str;
		if (m_connNumMesh != nullptr)
		{
			delete m_connNumMesh;
			m_connNumMesh = nullptr;
		}
		m_connNumMMin = m_addrMMin - Vector2(0.f, m_connNumMHeight);
		m_connNumMesh = Mesh::CreateTextImmediate(Rgba::WHITE, m_connNumMMin, font, 
			m_connNumMHeight, m_connNumMRatio, client_str, VERT_PCU);
		///////////////////////////////////////////// CONNECTION DETAIL MESH /////////////////////////////////////////////
		for (uint i = 0; i < m_connectMeshes.size(); ++i)
		{
			delete m_connectMeshes[i];
			m_connectMeshes[i] = nullptr;
		}
		m_connectMeshes.clear();

		for (uint i = 0; i < m_connections.size(); ++i)
		{
			std::string index_str = Stringf("[%i]", i);
			std::string status_str = "[JOINED]";			// only show "JOIN" for now
			std::string conn_str = m_connections[i]->m_address.ToString();
			m_connMMin = m_connNumMMin - Vector2(0.f, m_connMHeight * (i + 1));
			conn_str = index_str + conn_str + status_str;

			// all print white
			// todo: ANY in green?
			Mesh* m = Mesh::CreateTextImmediate(Rgba::WHITE, m_connMMin, font, 
				m_connMHeight, m_connMRatio, conn_str, VERT_PCU);
			m_connectMeshes.push_back(m);
		}
	}
		break;
	case RCS_HOST:
	{
		///////////////////////////////////////////// ADDR INFO MESH /////////////////////////////////////////////
		// i am host, addr text mesh should show this info
		std::string status = "[host]";

		// in host's case, join addr is same as host addr
		std::string join_addr = "join addr: " + m_listenSocket->m_address.ToString();

		if (m_addrMesh != nullptr)
		{
			delete m_addrMesh;
			m_addrMesh = nullptr;
		}

		Renderer* renderer = Renderer::GetInstance();
		m_addrMMin = m_titleMMin - Vector2(0.f, m_addrMHeight);
		BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");
		std::string addr_str = status + join_addr;

		// create updated addr mesh every frame
		m_addrMesh = Mesh::CreateTextImmediate(Rgba::WHITE, m_addrMMin, font, 
			m_addrMHeight, m_addrMRatio, addr_str, VERT_PCU);
		///////////////////////////////////////////// CONNECTION NUMBER MESH /////////////////////////////////////////////
		// i am host, i print client numbers
		std::string connect_count_str = std::to_string(m_connections.size());
		std::string max_connect_str = std::to_string(MAX_CONNECTIONS);
		std::string client_str = "client(s): " + connect_count_str + "/" + max_connect_str;

		if (m_connNumMesh != nullptr)
		{
			delete m_connNumMesh;
			m_connNumMesh = nullptr;
		}

		m_connNumMMin = m_addrMMin - Vector2(0.f, m_connNumMHeight);

		m_connNumMesh = Mesh::CreateTextImmediate(Rgba::WHITE, m_connNumMMin, font, 
			m_connNumMHeight, m_connNumMRatio, client_str, VERT_PCU);
		///////////////////////////////////////////// CONNECTION DETAIL MESH /////////////////////////////////////////////
		// construct connection meshes based on connection list
		for (uint i = 0; i < m_connectMeshes.size(); ++i)
		{
			delete m_connectMeshes[i];
			m_connectMeshes[i] = nullptr;
		}
		m_connectMeshes.clear();
		// now the slots are fresh and empty

		for (uint i = 0; i < m_connections.size(); ++i)
		{
			std::string index_str = Stringf("[%i]", i);
			std::string status_str = "[JOINED]";				// not closed == joined
			std::string conn_str = m_connections[i]->m_address.ToString();
			m_connMMin = m_connNumMMin - Vector2(0.f, m_connMHeight * (i + 1));
			conn_str = index_str + conn_str + status_str;

			// THEIR addr is same as MY addr: host self join, use a different color
			if ( m_listenSocket->m_address.ToString() == conn_str )
			{
				// print as green, as it is a self join
				Mesh* m = Mesh::CreateTextImmediate(Rgba::GREEN, m_connMMin, font, 
					m_connMHeight, m_connMRatio, conn_str, VERT_PCU);
				m_connectMeshes.push_back(m);

				continue;
			}

			// print as white for other connections
			Mesh* m = Mesh::CreateTextImmediate(Rgba::WHITE, m_connMMin, font, 
				m_connMHeight, m_connMRatio, conn_str, VERT_PCU);
			m_connectMeshes.push_back(m);
		}
	}
		break;
	case NUM_RCS_STATE:
		break;
	default:
		break;
	}
}

void RCS::Render(Renderer* renderer)
{
	if (m_titleMesh != nullptr)
		renderer->DrawMesh(m_titleMesh);
	if (m_addrMesh != nullptr)
		renderer->DrawMesh(m_addrMesh);
	if (m_connNumMesh != nullptr)
		renderer->DrawMesh(m_connNumMesh);
	if (!m_connectMeshes.empty())
	{
		for each (Mesh* m in m_connectMeshes)
		{
			renderer->DrawMesh(m);
		}
	}
}

void RCS::SendMsg(uint idx, uint8_t isEcho, const char* str)
{
	if (m_rcsState == RCS_HOST || m_rcsState == RCS_CLIENT)
	{
		LogTaggedPrintf("net", "host sending");

		BytePacker msg(BIG_ENDIAN);
		TCPSocket* sock = GetConnectionByIdx(idx);
		if (sock == nullptr) return;

		msg.WriteType<uint8_t>(isEcho);		// echo: 1, non_echo: 2
		msg.WriteString(str);
		msg.Seal();

		const size_t len = msg.GetWrittenByteCount();
		ASSERT_OR_DIE(len <= 0xffff, "Format does not support len larger than max ushort");

		uint16_t uslen = (uint16_t)len;			// not encoded yet					
		char* to_big_endian = new char[1024];					
		const uint8_t* buff = msg.GetBuffer();
		to_big_endian = ToEndianness(uslen, buff, msg.m_endian);		// todo: not flipping right now for some reason...

		size_t encoded_byte_num = msg.WriteSize(uslen);			// start encoding 
		std::vector<uint8_t> encoded_binary;
		for (int i = 0; i < encoded_byte_num; ++i)
		{
			uint8_t encoded_byte = msg.m_size[i];
			std::vector<uint8_t> binary = BinaryFromDecimal(encoded_byte);
			
			for (int j = 0; j < binary.size(); ++j)			// size of 8
				encoded_binary.push_back(binary[j]);
		}
		uint16_t encoded_size_two_bytes = (uint16_t)DecimalFromBinary(encoded_binary);

		sock->Send(encoded_size_two_bytes);			// templated overload
		sock->Send(to_big_endian, len+1);
	}
}

void RCS::ReceiveAndRunMsg(TCPSocket* connection)
{
	char len_buffer[2];
	char buffer[1024];
	size_t len_recvd = connection->Receive(len_buffer, 2);
	size_t recvd = connection->Receive(buffer, 1024 - 1U);
	if (recvd > 0U && len_recvd > 0U)
	{
		// get the true(decoded) size of string using unpacker
		BytePacker unpacker = BytePacker();
		unpacker.m_sizeOfHowManyBytes = 0;
		uint16_t encoded_size = BufferSizeHardCode16(len_buffer);
		std::vector<uint8_t> binaries = BinaryFromDecimal16(encoded_size);
		std::vector<uint8_t> used;
		bool non_chunking = false;
		for (int m = 7; m >= 0; --m)
		{
			int the_bit = binaries[m];
			if (the_bit == 1)
			{
				non_chunking = true;
				break;
			}
		}
		if (!non_chunking)
		{
			std::vector<uint8_t>::const_iterator first = binaries.begin() + 8;
			std::vector<uint8_t>::const_iterator last = binaries.end();
			std::vector<uint8_t> first_last(first, last);
			used = first_last;
		}
		else
			used = binaries;

		//uint8_t aByte;
		uint counter = 0;
		uint packerSizeCounter = 0;
		std::vector<uint8_t> bits;
		for each (uint8_t i in used)
		{
			bits.push_back(i);
			counter++;

			if (counter % 8 == 0)
			{
				uint8_t dec = (uint8_t)DecimalFromBinary(bits);
				unpacker.m_size[packerSizeCounter] = dec;
				packerSizeCounter++;

				bits.clear();

				unpacker.m_sizeOfHowManyBytes++;
			}
		}
		size_t true_size = unpacker.ReadSize();

		char* flipped_buffer = new char[true_size];
		flipped_buffer = FromEndianness(true_size, buffer, BIG_ENDIAN);

		// echo
		std::string char_str(flipped_buffer);
		uint8_t echo = (uint8_t)(BufferSizeHardCode16(char_str.substr(0, 1)));
		m_echo = echo;

		// cmd string
		std::string flipped_str(flipped_buffer);
		std::string cmd_str = flipped_str.substr(1, true_size - 1);

		bool cmd_run = CommandRun(cmd_str.c_str());
		if (!cmd_run)
		{
			ConsolePrintfUnit(Rgba::WHITE, cmd_str.c_str());
		}
	}
}

TCPSocket* RCS::GetConnectionByIdx(uint idx)
{
	if (!m_connections.empty())
	{
		return m_connections[idx];
	}
	
	return nullptr;
}

RCS* RCS::GetInstance()
{
	if (m_rcsInstance == nullptr)
	{
		m_rcsInstance = new RCS();

		// RCS kicked off
		// make sure we start with INITIAL_STATE
		m_rcsInstance->m_rcsState = RCS_INITIAL;
	}

	return m_rcsInstance;
}

void RCS::DestroyInstance()
{
	delete m_rcsInstance;
	m_rcsInstance = nullptr;
}

void RCS::Startup()
{
	// readyness
	std::string readyness = "Ready";
	std::string title = "Remote Command Service: [" + readyness + "]";

	Renderer* renderer = Renderer::GetInstance();
	DevConsole* console = DevConsole::GetInstance();
	m_titleMHeight = console->m_cameraOrtho.GetDimensions().y * 0.04f;
	float titleWidth = m_titleMRatio * m_titleMHeight * title.size();
	m_titleMMin = console->m_cameraOrtho.maxs - Vector2(titleWidth, m_titleMHeight);
	BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

	m_titleMesh = Mesh::CreateTextImmediate(Rgba::WHITE, m_titleMMin, font, 
		m_titleMHeight, m_titleMRatio, title, VERT_PCU);

	// other setups
	m_addrMHeight = console->m_cameraOrtho.GetDimensions().y * 0.02f;
	m_connNumMHeight = console->m_cameraOrtho.GetDimensions().y * 0.02f;
	m_connMHeight = console->m_cameraOrtho.GetDimensions().y * 0.02f;
}
