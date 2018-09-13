#include "Engine/Core/Log/LogSystem.hpp"
#include "Engine/Core/Log/FileUtils.hpp"
#include "Engine/Core/Thread/Thread.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

#include <stdarg.h>
#include <algorithm>  


sLogHook::sLogHook(LogCB cb, void* arg, bool output)
{
	m_callback = cb;
	m_userArg = arg;
	m_output = output;
}

sLogHook::~sLogHook()
{

}

// local
static LogSystem* g_LogSystem;
static File* g_LogFile = nullptr;
static File* g_DupFile = nullptr;

// thread call back
static void LogThreadWorker(void*)
{
	while (g_LogSystem->IsRunning())
	{
		g_LogSystem->Flush();
		ThreadSleep(10);
	}

	g_LogSystem->Flush();
}

static void LogFromFile(void*)
{
	File* file = new File("Data/Logs/Big.txt");

	// open the file in read-only
	file->ReadOpenFile(/*"Data/Logs/Big.txt"*/);

	std::string lineStr;
	while(file->Readline(lineStr))
	{
		LogTaggedPrintf("test", lineStr.c_str());
	}
	file->ReadCloseFile();

	delete file;
	file = nullptr;
}

// log call back 
static void WriteToFile(const sLog* log, void* filePtr)
{
	File* file = (File*)filePtr;			// safe cast

	bool blacklisted = (!g_LogSystem->m_whitelist) 
		&& (g_LogSystem->m_filters.Find(log->m_tag));
	bool unWhitelisted = (g_LogSystem->m_whitelist) 
		&& (!g_LogSystem->m_filters.Find(log->m_tag));

	if (!(blacklisted || unWhitelisted))
	{
		file->PrintFilef("%s, %s\n", log->m_tag.c_str(), log->m_text.c_str());
		//ConsolePrintfThreadSafe("%s, %s\n", log->m_tag.c_str(), log->m_text.c_str());		// same formatted msg
	}
}

static void OutputToPanel(const sLog*, void* data)
{
	const char* c = (const char*)data;
	DebuggerPrintf(c);
}

// start and shut down
void LogSystemStartup(const char* path)
{
	g_LogFile = new File(path);

	g_LogSystem = new LogSystem();
	g_LogSystem->m_thread = ThreadCreate(LogThreadWorker);
	g_LogSystem->m_running = true;

	if (g_LogFile->WriteOpenFile())
	{
		LogHook(WriteToFile, g_LogFile);
	}
	else
	{
		delete g_LogFile;
		g_LogFile = nullptr;
	}

	//g_LogSystem->OutputPanel("log system started up!");
}

void LogSystemShutdown()
{
	g_LogSystem->Stop();
	ThreadJoin(g_LogSystem->m_thread);

	g_LogSystem->m_thread = nullptr;

	if (g_LogFile != nullptr)
	{
		g_LogFile->WriteCloseFile();
		
		delete g_LogFile;
		g_LogFile = nullptr;
	}
}

// logging
void LogTaggedPrintv(const char* tag, const char* format, va_list args)
{
	if (g_LogSystem != nullptr && g_LogSystem->m_running)
	{
		sLog* log = new sLog();
		log->m_tag = tag;
		log->m_text = Stringv(format, args);

		g_LogSystem->m_logQueue.Enqueue(log);
	}
}


void LogTaggedPrintf(const char* tag, const char* format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv(tag, format, variableArgumentList);
}

// flush
void LogFlush()
{
	// the logger knows to flush
	// make sure that queue is empty
	g_LogSystem->Flush();
	g_LogFile->m_fs.flush();
}

// helper
void LogPrintv(const char* format, va_list args)
{
	sLog* log = new sLog();
	log->m_tag = "log";
	log->m_text = Stringv(format, args);

	g_LogSystem->m_logQueue.Enqueue(log);
}

void LogPrintf(const char* format, ...)
{
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogPrintv(format, variableArgumentList);
}

// warning and error
void LogWarningf(const char* format, ...)
{
	const char* tag = "warning";
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv(tag, format, variableArgumentList);
}

void LogErrorf(const char* format, ...)
{
	const char* tag = "error";
	va_list variableArgumentList;
	va_start(variableArgumentList, format);
	LogTaggedPrintv(tag, format, variableArgumentList);
}

// filtering
void LogShowAll()
{
	bool whitelist = g_LogSystem->m_whitelist;

	if (whitelist)
	{
		// whitelist, add all log tag to the list
		for (std::deque<sLog*>::size_type idx = 0;
			idx < g_LogSystem->m_logQueue.m_data.size(); ++idx)
		{
			sLog* log = g_LogSystem->m_logQueue.m_data[idx];

			std::string tag = log->m_tag;

			g_LogSystem->m_filters.Add(tag);
		}
	}
	else
	{
		// blacklist, show all means remove all tags in filter
		g_LogSystem->m_filters.RemoveAll();
	}
}

void LogHideAll()
{
	bool whitelist = g_LogSystem->m_whitelist;

	if (whitelist)
	{
		// whitelist, hide all means no tags in the filter
		g_LogSystem->m_filters.RemoveAll();
	}
	else
	{
		// blacklist, hide all means add all tags to the filter
		for (std::deque<sLog*>::size_type idx = 0;
			idx < g_LogSystem->m_logQueue.m_data.size(); ++idx)
		{
			sLog* log = g_LogSystem->m_logQueue.m_data[idx];

			std::string tag = log->m_tag;

			g_LogSystem->m_filters.Add(tag);
		}
	}
}

void LogShowTag(const char* tag)
{
	bool whitelist = g_LogSystem->m_whitelist;
	
	if (whitelist)
	{
		g_LogSystem->m_filters.Add(tag);
	}
	else
	{
		g_LogSystem->m_filters.Remove(tag);
	}
}

void LogHideTag(const char* tag)
{
	bool whitelist = g_LogSystem->m_whitelist;

	if (whitelist)
	{
		g_LogSystem->m_filters.Remove(tag);
	}
	else
	{
		g_LogSystem->m_filters.Add(tag);
	}
}

void EnableAll()
{
	g_LogSystem->m_whitelist = false;		// blacklist
	g_LogSystem->m_filters.RemoveAll();		// clear tags
}

void DisableAll()
{
	g_LogSystem->m_whitelist = true;		// whitelist
	g_LogSystem->m_filters.RemoveAll();		// clear tags
}

// hook and unhook
void LogHook(LogCB cb, void* userArg)
{
	g_LogSystem->m_hookLock.Enter();
	g_LogSystem->m_hooks.push_back(new sLogHook(cb, userArg));
	g_LogSystem->m_hookLock.Leave();
}

void LogUnhook(LogCB cb, void* userArg)
{
	g_LogSystem->m_hookLock.Enter();

	size_t subjectIdx = 0U;
	for (std::vector<sLogHook*>::size_type idx = 0;
		idx < g_LogSystem->m_hooks.size(); ++idx)
	{
		sLogHook* hook = g_LogSystem->m_hooks[idx];

		if (hook->m_callback == cb && hook->m_userArg == userArg)
		{
			subjectIdx = idx;
		}
	}
	g_LogSystem->m_hooks.erase(g_LogSystem->m_hooks.begin() + subjectIdx);
	
	g_LogSystem->m_hookLock.Leave();
}

// testing
void LogTest(uint threadCount)
{
	for (uint idx = 0; idx < threadCount; ++idx)
	{
		ThreadCreateAndDetach(LogFromFile);
	}
}

void LogFlushTest()
{
	LogShowTag("log");
	LogPrintf("log flush test");
	LogFlush();
	// breakpoint on this line; 

	std::string duplicatePath = Stringf("%s%s", DEFAULT_DUP_DIR, __TIME__); 
	
	if (g_DupFile != nullptr)
	{
		delete g_DupFile;
		g_DupFile = nullptr;
	}

	g_DupFile = new File(duplicatePath.c_str());

	if (g_DupFile->WriteOpenFile())
	{
		g_DupFile->m_fs << g_LogFile->m_fs.rdbuf();
	}
}

void LogConsoleTest()
{
	LogShowTag("console");
	LogTaggedPrintf("console", "console print test");
	//ConsolePrintfThreadSafe("console print test");
	//ConsolePrintf("console print test");
}

//////////////////////////////////////////////////// SYSTEM /////////////////////////////////////////////////////
bool LogSystem::IsRunning()
{
	return m_running;
}

void LogSystem::Stop()
{
	m_running = false;
}

void LogSystem::Flush()
{
	sLog* log;
	while (m_logQueue.Dequeue(&log))
	{
		// iterate hooks and activate callback
		m_hookLock.Enter();
		for each (sLogHook* hook in m_hooks)
		{
			hook->m_callback(log, hook->m_userArg);
		}
		m_hookLock.Leave();

		delete log;
	}
}

void LogSystem::OutputPanel(void* msg)
{
	LogHook(OutputToPanel, msg);
}