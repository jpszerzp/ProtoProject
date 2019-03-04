#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/Command.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/Thread/Thread.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Log/LogSystem.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Net/NetAddress.hpp"
#include "Engine/Net/TCPSocket.hpp"
#include "Engine/Net/RemoteCommandService.hpp"
#include "Engine/Net/Socket.hpp"
#include "Engine/Math/QuickHull.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

#include <limits.h>
#include <stdexcept>      // std::invalid_argument

class CommandDef
{
public:
	CommandDef()
	{
		m_name = "";
		m_cb = nullptr;
	}

	CommandDef(std::string theName, command_cb theCB, std::string argDoc, std::string cmdDoc)
	{
		m_name = theName;
		m_cb = theCB;
		m_argumentDoc = argDoc;
		m_cmdDoc = cmdDoc;
	}

	std::string m_name;
	std::string m_argumentDoc;
	std::string m_cmdDoc;
	command_cb m_cb;
};


CommandDef gDef[128];
int gDefCount = 0;

/*
void RCCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();

	console->ClearOutputBufferFormat();

	std::string errorMsg = "rc error: NOT IMPLEMENTED, added to test auto complete";
	Rgba errorColor = Rgba::RED;
	console->AppendFormatToOutputBuffer(errorMsg);
	//console->FillOutputBuffer(errorColor);

	g_rcs->m_echo = 2;
}


void RCACommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();

	console->ClearOutputBufferFormat();

	std::string errorMsg = "rca error: NOT IMPLEMENTED, added to test auto complete";
	Rgba errorColor = Rgba::RED;
	console->AppendFormatToOutputBuffer(errorMsg);
	//console->FillOutputBuffer(errorColor);

	g_rcs->m_echo = 2;
}


void JoinCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();

	console->ClearOutputBufferFormat();

	std::string errorMsg = "rcs_join error: NOT IMPLEMENTED, added to test auto complete";
	Rgba errorColor = Rgba::RED;
	console->AppendFormatToOutputBuffer(errorMsg);
	//console->FillOutputBuffer(errorColor);

	g_rcs->m_echo = 2;
}


void StartCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();

	console->ClearOutputBufferFormat();

	std::string errorMsg = "rcs_start error: NOT IMPLEMENTED, added to test auto complete";
	Rgba errorColor = Rgba::RED;
	console->AppendFormatToOutputBuffer(errorMsg);
	//console->FillOutputBuffer(errorColor);

	g_rcs->m_echo = 2;
}
*/

void SaveLogCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	std::string fileName = cmd.GetNextString();
	if (fileName.find('\"') != std::string::npos)
	{
		fileName = fileName.substr(1, fileName.length() - 2);
		fileName = "Data/Logs/" + fileName;
		BufferWriteToFile(fileName.c_str());
		console->ClearOutputBufferFormat();

		std::string successMsg = "Log saved";
		Rgba successColor = Rgba::WHITE;
		//console->AppendFormatToOutputBuffer(successMsg);
		//console->FillOutputBuffer(successColor);
		ConsolePrintfUnit(successColor, successMsg.c_str());
	}
	else
	{
		console->ClearOutputBufferFormat();

		std::string errorMsg = "save_log error: INVALID ARGUMENT, no quoted string received for file path";
		Rgba errorColor = Rgba::RED;
		//console->AppendFormatToOutputBuffer(errorMsg);
		//console->FillOutputBuffer(errorColor);
		ConsolePrintfUnit(errorColor, errorMsg.c_str());
	}

	g_rcs->m_echo = 2;
}


void EchoWithColorCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	std::string color_str = cmd.GetNextString();	// in form of r, g, b
	std::string printed_str = cmd.GetNextString();
	Rgba outputColor;

	if (color_str.empty())
	{
		color_str = "echo_with_color error: INVALID ARGUMENT, no color provided";
		outputColor = Rgba::RED;
		//console->AppendFormatToOutputBuffer(color_str);
		//console->FillOutputBuffer(outputColor);
		ConsolePrintfUnit(outputColor, color_str.c_str());
		return;
	}
	else
	{
		outputColor.SetFromText(color_str.c_str());
	}

	if (printed_str.find('\"') != std::string::npos)
	{
		printed_str = printed_str.substr(1, printed_str.length() - 2);
	}
	else
	{
		printed_str = "echo_with_color error: INVALID ARGUMENT, no quoted string received";
		outputColor = Rgba::RED;
		//console->AppendFormatToOutputBuffer(printed_str);
		//console->FillOutputBuffer(outputColor);
		ConsolePrintfUnit(outputColor, printed_str.c_str());
		return;
	}

	//console->AppendFormatToOutputBuffer(printed_str);
	//console->FillOutputBuffer(outputColor);
	ConsolePrintfUnit(outputColor, printed_str.c_str());

	g_rcs->m_echo = 2;
}


void HistoryCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	std::vector<std::string>& histories = console->GetCmdHistory();
	for each (std::string history in histories)
	{
		//ConsolePrintf(history.c_str());
		ConsolePrintfUnit(Rgba::WHITE, history.c_str());
	}

	//Rgba tint = Rgba::WHITE;
	//console->FillOutputBuffer(tint);

	g_rcs->m_echo = 2;
}


void QuitCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();
	console->SetAppShouldQuit(true);

	g_rcs->m_echo = 2;
}


void ClearCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	ConsolePrintfUnit(Rgba::WHITE, "Cleared");

	g_rcs->m_echo = 2;
}


void HelpCommand ( Command &cmd )
{
	DevConsole* console = DevConsole::GetInstance();

	console->ClearOutputBufferFormat();
	std::string subStr = cmd.GetNextString();

	if (subStr.empty())
	{
		for (int cmdCount = 0; cmdCount < gDefCount; ++cmdCount)
		{
			const char* cmdName = (gDef[cmdCount].m_name).c_str();
			const char* argDoc = (gDef[cmdCount].m_argumentDoc).c_str();
			const char* cmdDoc = (gDef[cmdCount].m_cmdDoc).c_str();
			ConsolePrintfUnit(Rgba::WHITE, "%s %s: %s", cmdName, argDoc, cmdDoc);
		}
	}
	else
	{
		int foundWords = 0;

		for (int cmdCount = 0; cmdCount < gDefCount; ++cmdCount)
		{
			CommandDef* def = gDef + cmdCount;

			if (def->m_name == subStr)
			{
				const char* cmdName = (gDef[cmdCount].m_name).c_str();
				const char* argDoc = (gDef[cmdCount].m_argumentDoc).c_str();
				const char* cmdDoc = (gDef[cmdCount].m_cmdDoc).c_str();
				ConsolePrintfUnit(Rgba::WHITE, "%s %s: %s", cmdName, argDoc, cmdDoc);

				foundWords++;
			}
		}

		if (foundWords == 0)
		{
			ConsolePrintfUnit(Rgba::RED, "help error: INVALID COMMAND");
			return;
		}
	}

	g_rcs->m_echo = 2;
}


void DebugRenderCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();

	console->ClearOutputBufferFormat();
	std::string subStr = cmd.GetNextString();
	Rgba outputColor;

	if (subStr == "true")
	{
		SetDebugRenderOn(true);
		ConsolePrintfUnit(Rgba::GREEN, "Debug render mode turned on!");
	}
	else if (subStr == "false")
	{
		SetDebugRenderOn(false);
		ConsolePrintfUnit(Rgba::YELLOW, "Debug render mode turned off!");
	}
	else
	{
		ConsolePrintfUnit(Rgba::RED, "Unknown command in setting debug render mode!");
		return;
	}

	g_rcs->m_echo = 2;
}


void ThreadTestCommand(Command&)
{
	ThreadCreateAndDetach(ThreadTest, nullptr);

	g_rcs->m_echo = 2;
}


void NonThreadTestCommand(Command&)
{
	ThreadTest(nullptr);

	g_rcs->m_echo = 2;
}


void LogTestCommand(Command&)
{
	LogTest(4);
	g_rcs->m_echo = 2;
}


void LogFlushCommand(Command&)
{
	LogFlushTest();
	g_rcs->m_echo = 2;
}

void LogShowAllCommand(Command&)
{
	LogShowAll();
	g_rcs->m_echo = 2;
}

void LogHideAllCommand(Command&)
{
	LogHideAll();
	g_rcs->m_echo = 2;
}

void LogShowTagCommand(Command& cmd)
{
	//DevConsole::GetInstance()->ClearOutputBufferFormat();
	std::string subStr = cmd.GetNextString();
	LogShowTag(subStr.c_str());
	g_rcs->m_echo = 2;
}

void LogHideTagCommand(Command& cmd)
{
	//DevConsole::GetInstance()->ClearOutputBufferFormat();
	std::string subStr = cmd.GetNextString();
	LogHideTag(subStr.c_str());
	g_rcs->m_echo = 2;
}

void ThreadPrintCommand(Command&)
{
	LogConsoleTest();
	g_rcs->m_echo = 2;
}


void NetLocalCommand(Command&)
{
	char out[256]; 

	GetAddressString(out);

	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	ConsolePrintfUnit(Rgba::WHITE, "My address: %s", out);

	g_rcs->m_echo = 2;
}

void ConnectTestCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	std::string addr_str = cmd.GetNextString();
	const char* msg_str = "john peng";

	// blocking socket, not using TCPSocket
	Connect(sNetAddress(addr_str.c_str()), msg_str);

	g_rcs->m_echo = 2;
}

void TCPConnectTestCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	std::string addr_str = cmd.GetNextString();
	const char* msg_str = "john peng";

	// can choose blockness, use TCPSocket
	TCPConnect(sNetAddress(addr_str.c_str()), msg_str);

	g_rcs->m_echo = 2;
}

void ServerTestCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	uint16_t port = 0;
	std::string port_str = cmd.GetNextString();
	port = (uint16_t)std::stoi(port_str);

	Host(port);

	g_rcs->m_echo = 2;
}

void TCPServerTestCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	uint16_t port = 0;
	std::string port_str = cmd.GetNextString();
	port = (uint16_t)std::stoi(port_str);

	TCPHost(port);

	g_rcs->m_echo = 2;
}

void RemoteCommand(Command& cmd)
{
	//DevConsole* console = DevConsole::GetInstance();
	//console->ClearOutputBufferFormat();

	int clientIdx = 0;				// default to client index 0 
	std::string indexStr;
	std::string cmdStr;
	try
	{
		indexStr = cmd.GetNextString();
		clientIdx = std::stoi(indexStr);

		cmdStr = cmd.GetNextString();

		// idx is provided; echo false since we didn't use echo cmd
		//g_rcs->SendMsg(clientIdx, 1, cmdStr.c_str());
		g_rcs->SendMsg(clientIdx, 2, cmdStr.c_str());
	}
	catch (std::invalid_argument&)
	{
		// so, idx is 0; echo is false since we didn't use echo cmd
		//g_rcs->SendMsg(0, 1, indexStr.c_str());
		g_rcs->SendMsg(0, 2, indexStr.c_str());
	}

	g_rcs->m_echo = 2;
}

void RemoteBroadcastCommand(Command& cmd)
{
	//DevConsole* console = DevConsole::GetInstance();
	//console->ClearOutputBufferFormat();

	std::string cmdStr;
	cmdStr = cmd.GetNextString();
	
	if (g_rcs->m_rcsState == RCS_CLIENT)
		g_rcs->SendMsg(0, 2, cmdStr.c_str());	// only send to host
	else if (g_rcs->m_rcsState == RCS_HOST)
	{
		for (int i = 0; i < g_rcs->m_connections.size(); ++i)
		{
			g_rcs->SendMsg(i, 2, cmdStr.c_str());
		}
	}

	g_rcs->m_echo = 2;
}

void RemoteAllCommand(Command& cmd)
{
	std::string cmdStr;
	cmdStr = cmd.GetNextString();

	if (g_rcs->m_rcsState == RCS_CLIENT)
	{
		g_rcs->SendMsg(0, 2, cmdStr.c_str());	// only send to host
		CommandRun(cmdStr.c_str());
	}
	else if (g_rcs->m_rcsState == RCS_HOST)
	{
		for (int i = 0; i < g_rcs->m_connections.size(); ++i)
		{
			g_rcs->SendMsg(i, 2, cmdStr.c_str());
		}
		CommandRun(cmdStr.c_str());
	}

	g_rcs->m_echo = 2;
}


void RemoteJoinCommand(Command& cmd)
{
	std::string addr_port_str = cmd.GetNextString();
	
	if (g_rcs->m_rcsState == RCS_CLIENT)
	{
		// get host connection
		TCPSocket* to_host = g_rcs->m_connections[0];

		if (to_host != nullptr)
		{
			to_host->Close();

			delete to_host;
			to_host = nullptr;
		}
		g_rcs->m_connections.clear();
		// now the connection to original host is off
		ConsolePrintfUnit(Rgba::GREEN, "rc_join: Client disconnected");

		// attempt to join new host
		sNetAddress saddr = sNetAddress(addr_port_str.c_str());
		TCPSocket* join_other = new TCPSocket();
		bool other_joined = join_other->Connect(saddr);
		//join_other->SetBlockMode(false);
		if (other_joined)
		{
			// joined another host
			g_rcs->m_connections.push_back(join_other);
			join_other->SetBlockMode(false);

			ConsolePrintfUnit(Rgba::GREEN, "New host joined");
			g_rcs->m_rcsState = RCS_CLIENT;
		}
		else
		{
			// new host join fails
			// back to normal flow
			ConsolePrintfUnit(Rgba::RED, "New host join failed");
			g_rcs->m_rcsState = RCS_INITIAL;
			
			delete join_other;
		}
	}
	else if (g_rcs->m_rcsState == RCS_HOST)
	{
		// stop hosting
		// close connections to clients
		for each (TCPSocket* to_client in g_rcs->m_connections)
		{
			to_client->Close();

			delete to_client;
			to_client = nullptr;
		}
		g_rcs->m_connections.clear();

		// close listen socket
		g_rcs->m_listenSocket->Close();
		delete g_rcs->m_listenSocket;
		g_rcs->m_listenSocket = nullptr;
		ConsolePrintfUnit(Rgba::GREEN, "rc_join: Host disconnected");

		// try to connect to provided addr
		sNetAddress saddr = sNetAddress(addr_port_str.c_str());
		TCPSocket* join_other = new TCPSocket();
		bool other_joined = join_other->Connect(saddr);
		if (other_joined)
		{
			// joined another host
			g_rcs->m_connections.push_back(join_other);
			join_other->SetBlockMode(false);

			ConsolePrintfUnit(Rgba::GREEN, "New host joined");
			g_rcs->m_rcsState = RCS_CLIENT;
		}
		else
		{
			// new host join fails
			// back to normal flow
			ConsolePrintfUnit(Rgba::RED, "New host join failed");
			g_rcs->m_rcsState = RCS_INITIAL;

			delete join_other;
		}
	}

	g_rcs->m_echo = 2;
}

void RemoteHostCommand(Command& cmd)
{
	uint16_t usedPort = RCS::m_port;
	
	std::string port_str = cmd.GetNextString();

	if (!port_str.empty())
		usedPort = (uint16_t)(std::stoi(port_str));
	// confirmed on host port 

	if (g_rcs->m_rcsState == RCS_HOST)
	{
		// stop current hosting
		// close connections to clients
		for each (TCPSocket* to_client in g_rcs->m_connections)
		{
			to_client->Close();

			delete to_client;
			to_client = nullptr;
		}
		g_rcs->m_connections.clear();

		// close listen socket
		g_rcs->m_listenSocket->Close();
		delete g_rcs->m_listenSocket;
		g_rcs->m_listenSocket = nullptr;
		ConsolePrintfUnit(Rgba::GREEN, "rc_host: Host disconnected");

		// try to host on given port with local ip
		sNetAddress addr;
		if (!sNetAddress::GetBindableAddr(&addr, usedPort))
		{
			ConsolePrintfUnit(Rgba::RED, "rc_host: cannot resolve ip with given port");
			g_rcs->m_rcsState = RCS_INITIAL;
		}
		else
		{
			// we resovled new host addr, start hosting
			g_rcs->m_listenSocket = new TCPSocket();

			int max_queued = 16;
			bool host_listened = g_rcs->m_listenSocket->Listen(usedPort, max_queued);

			if (!host_listened)
			{
				ConsolePrintfUnit(Rgba::RED, "rc_host: Host failed");

				delete g_rcs->m_listenSocket;
				g_rcs->m_listenSocket = nullptr;
				g_rcs->m_rcsState = RCS_INITIAL;
			}
			else
			{
				ConsolePrintfUnit(Rgba::GREEN, "rc_host: Host succeeds");
				g_rcs->m_listenSocket->SetBlockMode(false);
				g_rcs->m_rcsState = RCS_HOST;
			}
		}
	}
	else if (g_rcs->m_rcsState == RCS_CLIENT)
	{
		// get host connection, disconnect
		TCPSocket* to_host = g_rcs->m_connections[0];

		if (to_host != nullptr)
		{
			to_host->Close();

			delete to_host;
			to_host = nullptr;
		}
		g_rcs->m_connections.clear();
		// now the connection to original host is off
		ConsolePrintfUnit(Rgba::GREEN, "rc_host: Client disconnected");

		sNetAddress addr;
		if (!sNetAddress::GetBindableAddr(&addr, usedPort))
		{
			ConsolePrintfUnit(Rgba::RED, "rc_host: cannot resolve ip with given port");
			g_rcs->m_rcsState = RCS_INITIAL;
		}
		else
		{
			// we resovled new host addr, start hosting
			g_rcs->m_listenSocket = new TCPSocket();

			int max_queued = 16;
			bool host_listened = g_rcs->m_listenSocket->Listen(usedPort, max_queued);

			if (!host_listened)
			{
				ConsolePrintfUnit(Rgba::RED, "rc_host: Host failed");

				delete g_rcs->m_listenSocket;
				g_rcs->m_listenSocket = nullptr;
				g_rcs->m_rcsState = RCS_INITIAL;
			}
			else
			{
				ConsolePrintfUnit(Rgba::GREEN, "rc_host: Host succeeds");
				g_rcs->m_listenSocket->SetBlockMode(false);
				g_rcs->m_rcsState = RCS_HOST;
			}
		}
	}

	g_rcs->m_echo = 2;
}

void RemoteEchoCommand(Command&)
{
	g_rcs->m_echo = 2;
}

void SpawnProcessCommand(Command& cmd)
{
	int usedNum = 1;

	std::string num_str = cmd.GetNextString();

	if (!num_str.empty())
		usedNum = (uint16_t)(std::stoi(num_str));

	wchar_t buffer[MAX_PATH]; 
	::GetModuleFileName(NULL, buffer, MAX_PATH);

	ConsolePrintfUnit(Rgba::WHITE, (const char*)buffer);

	for (int i = 0; i < usedNum; ++i)
	{

		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		LPTSTR cmd_line = GetCommandLine();

		// Start the child process. 
		if( !::CreateProcess( buffer,   // No module name (use command line)
			cmd_line,        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			TRUE,           // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi )           // Pointer to PROCESS_INFORMATION structure
			) 
		{
			printf( "CreateProcess failed (%d).\n", GetLastError() );
			return;
		}

		// COMMENTED: keep the parent running 
		// Wait until child process exits.
		//::WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		//::CloseHandle( pi.hProcess );
		//::CloseHandle( pi.hThread );
	}

	g_rcs->m_echo = 2;
}

void UDPStartCommand(Command&)
{
	UDPTest* udp = UDPTest::GetInstance();
	udp->Start();
	ConsolePrintfUnit(Rgba::GREEN, "UDP starting!");
}

void UDPStopCommand(Command&)
{
	UDPTest* udp = UDPTest::GetInstance();
	udp->Stop();
	ConsolePrintfUnit(Rgba::GREEN, "UDP stopping!");
}

void UDPSendCommand(Command& cmd)
{
	UDPTest* udp = UDPTest::GetInstance();
	std::string addr_str = cmd.GetNextString();
	sNetAddress addr = sNetAddress(addr_str.c_str());
	std::string msg = "udp test";
	udp->SendTo(addr, "udp test", StringGetSize(msg.c_str()));
	ConsolePrintfUnit(Rgba::GREEN, "UDP sending");
}

void BroadPhaseCommand(Command& cmd)
{
	std::string subStr = cmd.GetNextString();

	if (subStr == "true")
	{
		g_broadphase = true;
		ConsolePrintfUnit(Rgba::GREEN, "broad phase on!");
	}
	else if (subStr == "false")
	{
		g_broadphase = false;
		ConsolePrintfUnit(Rgba::YELLOW, "broad phase off!");
	}
	else
		ConsolePrintfUnit(Rgba::RED, "Unknown arg!");
}

void RestitutionCommand(Command& cmd)
{
	std::string subStr = cmd.GetNextString();

	float r = std::stof(subStr);

	g_col_keep->m_global_restitution = r;

	ConsolePrintfUnit(Rgba::GREEN, "Restitution set to %f", r);
}

/*
void QHNormalDrawCommand(Command& cmd)
{
	std::string subStr = cmd.GetNextString();

	if (subStr == "true")
	{
		g_hull->CreateNormalMeshes();
		ConsolePrintfUnit(Rgba::GREEN, "QH normal meshes generated!");
	}
	else if (subStr == "false")
	{
		g_hull->FlushNormalMeshes();
		ConsolePrintfUnit(Rgba::YELLOW, "QH normal meshes flushed!");
	}
}
*/

void EncounterCommand(Command&)
{
	g_rcs->m_echo = 2;
}

Command::Command(const char* str)
{
	m_orderOfProcessedStr = 0;

	std::string theStr(str);
	m_strContent = theStr;
	size_t index = theStr.find(" ");
	if (index == std::string::npos)
	{
		m_cmdName = theStr;
	}
	else 
	{
		m_cmdName = theStr.substr(0, index);
	}
}


Command::~Command()
{
	
}


std::string Command::GetName()
{
	return m_cmdName;
}


std::string Command::GetNextString()
{
	m_orderOfProcessedStr++;

	std::string str = m_strContent;
	int nthIndex = nth_match_str(str, " ", m_orderOfProcessedStr);
	int nextStrStartIndex;
	if (nthIndex == INT_MAX)
	{
		m_orderOfProcessedStr = 0;
		DebuggerPrintf("you have exceeded the end of arg list!\n");
		return "";
	}
	else if (nthIndex == -1)
	{
		m_orderOfProcessedStr = 0;
		DebuggerPrintf("no arg is found\n");
		return "";
	}
	else
	{
		nextStrStartIndex = nthIndex + 1;
		if (nextStrStartIndex >= m_strContent.length())
		{
			m_orderOfProcessedStr = 0;
			DebuggerPrintf("you have exceeded the end of arg list!\n");
			return "";
		}
		else 
		{
			std::string remains = m_strContent.substr(nextStrStartIndex);
			size_t nextSpacePos = remains.find(' ');
			std::string remains_first_half = remains.substr(0, nextSpacePos);
			if (remains_first_half.find('\"') != std::string::npos)
			{
				std::string remains_second_half = remains.substr(nextSpacePos + 1);
				if (remains_second_half == remains_first_half)
				{
					return remains_first_half;
				}
				size_t closingQuotePos = remains_second_half.find('\"');
				std::string quotedStr = remains.substr(1, (nextSpacePos - 1) + 1 + closingQuotePos);
				size_t numSpace = std::count(quotedStr.begin(), quotedStr.end(), ' ');
				m_orderOfProcessedStr += (int)numSpace;
				quotedStr = '\"' + quotedStr + '\"';
				return quotedStr;
			}
			else
			{
				return remains_first_half;
			}
		}
	}
}


void CommandStartup()
{
	CommandRegister("help", HelpCommand, "{command}", "List specified command; if none specified, list all.");
	CommandRegister("clear", ClearCommand, "{}", "Clear the developer console.");
	CommandRegister("quit", QuitCommand, "{}", "Quit the application.");
	CommandRegister("save_log", SaveLogCommand, "{string}", "Takes a filename, and saves a copy of current console output to the file.");
	CommandRegister("echo_with_color", EchoWithColorCommand, "{color,string}", "Prints the supplied text with the supplied color to the console.");
	CommandRegister("history", HistoryCommand, "{}", "Prints input history recorded in history log.");

	CommandRegister("load_encounter", EncounterCommand, "{string}", "Receive encounter name data to load an encounter.");
	CommandRegister("render_debug", DebugRenderCommand, "{bool}", "Turn on Debug Render mode if true; turn off otherwise.");

	CommandRegister("thread_test", ThreadTestCommand, "{}", "Test thread with a slow operation by having no hitch");
	CommandRegister("non_thread_test", NonThreadTestCommand, "{}", "Test thread with a slow operation by having hitch");
	CommandRegister("log_test", LogTestCommand, "{}", "Test threaded log system");
	CommandRegister("log_flush", LogFlushCommand, "{}", "Testing flush on log message queue");
	CommandRegister("log_show_all", LogShowAllCommand, "{}", "Show logs with tags");
	CommandRegister("log_hide_all", LogHideAllCommand, "{}", "Hide logs with tags");
	CommandRegister("log_show", LogShowTagCommand, "{string}", "Always show logs with specified tag");
	CommandRegister("log_hide", LogHideTagCommand, "{string}", "Always hide logs with specified tag");
	CommandRegister("thread_print", ThreadPrintCommand, "{}", "Demonstrate threaded console print");

	// network
	CommandRegister("net_print_local_addr", NetLocalCommand, "{}", "Print local IP");
	CommandRegister("connect_test", ConnectTestCommand, "{string}", "Test connection given addr");
	CommandRegister("tcp_connect_test", TCPConnectTestCommand, "{string}", "Test connection with tcp socket given addr");
	CommandRegister("server_test", ServerTestCommand, "{int}", "Test server given port number");
	CommandRegister("tcp_server_test", TCPServerTestCommand, "{int}", "Test server with self-defined tcp sockets given port number");

	// remote
	CommandRegister("rc", RemoteCommand, "{int, command}", "Send a command to the connection associated with the index");
	CommandRegister("rcb", RemoteBroadcastCommand, "{command}", "Send a command to all connections I know about, but do not run the command locally");
	CommandRegister("rca", RemoteAllCommand, "{command}", "Send a command to all connections, and run it locally");
	CommandRegister("rc_join", RemoteJoinCommand, "{string, int}", "Leave current service, and attempt to joint his address instead of local");
	CommandRegister("rc_host", RemoteHostCommand, "{int}", "Leave the current service, and attempt to host on the given port");
	CommandRegister("rc_echo", RemoteEchoCommand, "{bool}", "All echoed responses are printed by default");

	CommandRegister("spawn_process", SpawnProcessCommand, "{}", "Spawn a copy of current process");

	// udp test
	CommandRegister("udp_start", UDPStartCommand, "{}", "Starting UDP test");
	CommandRegister("udp_stop", UDPStopCommand, "{}", "Stopping UDP test");
	CommandRegister("udp_send", UDPSendCommand, "{}", "Sending with UDP");
	
	// physics
	CommandRegister("broadphase", BroadPhaseCommand, "{bool}", "Turn on/off broadphase");
	//CommandRegister("qh_draw_normal", QHNormalDrawCommand, "{true}", "Draw face normal of a convex hull");
	CommandRegister("set_rst", RestitutionCommand, "{float}", "Set global restitution to specified value");
}


void CommandShutdown()
{
	for (int cmdDefCount = 0; cmdDefCount < 128; ++cmdDefCount)
	{
		// empty command def
		gDef[cmdDefCount] = CommandDef();			
	}

	gDefCount = 0;
}


void CommandRegister( char const *name, command_cb cb, const char* argDoc, const char* cmdDoc )
{
	gDef[gDefCount] = CommandDef(name, cb, argDoc, cmdDoc);
	gDefCount++;
}


CommandDef* CommandFind(std::string name)
{
	for (int commandCount = 0; commandCount < gDefCount; ++commandCount)
	{
		if (gDef[commandCount].m_name == name)
		{
			return &gDef[commandCount];
		}
	}

	return nullptr;
}


bool CommandRun(const char* command)
{
	Command cmd(command);
	std::string name = cmd.GetName();
	CommandDef* cmdDef = CommandFind(name);
	if (cmdDef != nullptr)
	{
		cmdDef->m_cb(cmd);
		return true;
	}
	else
	{
		DevConsole* console = DevConsole::GetInstance();
		console->ClearOutputBufferFormat();

		const char* errorText = "error: INVALID INPUT";
		ConsolePrintfUnit(Rgba::RED, errorText);

		return false;
	}
}


std::vector<std::string> GetCommandHistory()
{
	DevConsole* console = DevConsole::GetInstance();
	return console->GetCmdHistory();
}


TODO("Cmd extra - run script");
void CommandRunScript( const char* )
{

}


TODO("Cmd extra - run script to file");
void CommandRunScriptFile( const char* )
{

}


std::vector<std::string> CommandAutoComplete( char const *root )
{
	std::vector<std::string> matchRes;
	std::string rootStr(root);

	if (rootStr.empty())
	{
		return matchRes;
	}
	else
	{
		for (int cmdCount = 0; cmdCount < gDefCount; ++cmdCount)
		{
			CommandDef cmdDef = gDef[cmdCount];

			if (cmdDef.m_name.find(rootStr) != std::string::npos)
			{
				size_t foundPos = cmdDef.m_name.find(rootStr);
				if (foundPos == 0)
				{
					std::string cmdName = cmdDef.m_name;
					matchRes.push_back(cmdName);
				}
			}
		}
	}

	return matchRes;
}


std::string FindCmdDocWithName(std::string name)
{
	std::string res;

	for (int cmdCount = 0; cmdCount < gDefCount; ++cmdCount)
	{
		CommandDef* def = gDef + cmdCount;

		if ((def->m_name).find(name) != std::string::npos)
		{
			res = def->m_cmdDoc;
			break;
		}
	}

	return res;
}