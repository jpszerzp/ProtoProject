#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/Command.hpp"
#include "Engine/Core/Console/DevConsole.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/Util/AssetUtils.hpp"
#include "Engine/Core/Thread/Thread.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"

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

void SaveLogCommand(Command& cmd)
{
	DevConsole* console = DevConsole::GetInstance();
	std::string fileName = cmd.GetNextString();
	if (fileName.find('\"') != std::string::npos)
	{
		fileName = fileName.substr(1, fileName.length() - 2);
		fileName = GetAbsLogPath(fileName);
		BufferWriteToFile(fileName.c_str());
		console->ClearOutputBufferFormat();

		std::string successMsg = "Log saved";
		Rgba successColor = Rgba::WHITE;
		ConsolePrintfUnit(successColor, successMsg.c_str());
	}
	else
	{
		console->ClearOutputBufferFormat();

		std::string errorMsg = "save_log error: INVALID ARGUMENT, no quoted string received for file path";
		Rgba errorColor = Rgba::RED;
		ConsolePrintfUnit(errorColor, errorMsg.c_str());
	}
}

void HistoryCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	std::vector<std::string>& histories = console->GetCmdHistory();
	for each (std::string history in histories)
	{
		ConsolePrintfUnit(Rgba::WHITE, history.c_str());
	}
}


void QuitCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();
	console->SetAppShouldQuit(true);
}


void ClearCommand(Command&)
{
	DevConsole* console = DevConsole::GetInstance();
	console->ClearOutputBufferFormat();

	ConsolePrintfUnit(Rgba::WHITE, "Cleared");
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
}

void ThreadTestCommand(Command&)
{
	ThreadCreateAndDetach(ThreadTest, nullptr);
}

void NonThreadTestCommand(Command&)
{
	ThreadTest(nullptr);
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
	CommandRegister("history", HistoryCommand, "{}", "Prints input history recorded in history log.");
	CommandRegister("render_debug", DebugRenderCommand, "{bool}", "Turn on Debug Render mode if true; turn off otherwise.");
	CommandRegister("thread_test", ThreadTestCommand, "{}", "Test thread with a slow operation by having no hitch");
	CommandRegister("non_thread_test", NonThreadTestCommand, "{}", "Test thread with a slow operation by having hitch");
	CommandRegister("spawn_process", SpawnProcessCommand, "{}", "Spawn a copy of current process");
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