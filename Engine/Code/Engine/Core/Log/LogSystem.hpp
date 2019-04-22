#pragma once

#include "Engine/Core/Thread/Thread.hpp"
#include "Engine/Core/Thread/ThreadSafeQueue.hpp"
#include "Engine/Core/Thread/ThreadSafeSet.hpp"
#include "Engine/Core/Thread/SpinLock.hpp"

#define DEFAULT_LOG_NAME "Data/Logs/log.txt"
#define DEFAULT_DUP_DIR "Data/Logs/log_"

struct sLog
{
	std::string m_tag;
	std::string m_text;
};

typedef void (*LogCB)(const sLog* log, void* arg);

struct sLogHook
{
	LogCB m_callback;
	void* m_userArg;
	bool  m_output = false;

	sLogHook(LogCB cb, void* arg, bool output = false);
	~sLogHook();
};

// spins up and shut down
void LogSystemStartup(const char* root = DEFAULT_LOG_NAME);
void LogSystemShutdown();

// logging call
void LogTaggedPrintv(const char* tag, const char* format, va_list args);
void LogTaggedPrintf(const char* tag, const char* format, ...);

// flush
void LogFlush();

// helper
void LogPrintv(const char* format, va_list args);
void LogPrintf(const char* format, ...);

// warning and error
void LogWarningf(const char* format, ...);
void LogErrorf(const char* format, ...);

// filtering
void LogShowAll();
void LogHideAll();
void LogShowTag(const char* tag);
void LogHideTag(const char* tag);
void EnableAll();
void DisableAll();

// hooking
void LogHook(LogCB cb, void* userArg = nullptr);
void LogUnhook(LogCB cb, void* userArg = nullptr);

// test
void LogTest(uint threadCount);
void LogFlushTest();
void LogConsoleTest();

class LogSystem
{
public:
	tThreadHandle				 m_thread;
	bool						 m_running;
	ThreadSafeQueue<sLog*>		 m_logQueue;
	SpinLock					 m_hookLock;
	std::vector<sLogHook*>		 m_hooks;
	ThreadSafeSet<std::string>	 m_filters;
	bool						 m_whitelist = false;	// blacklist by default

public:
	bool IsRunning();
	void Stop();
	void Flush();

	// utilities
	void OutputPanel(void* msg);
};