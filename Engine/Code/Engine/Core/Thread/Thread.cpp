#include "Engine/Core/Thread/Thread.hpp"
#include "Engine/Core/Log/FileUtils.hpp"
#include "Engine/Core/Util/AssetUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <windows.h>
#include <process.h>
#include <fstream>
#include <iostream>

#define MS_VC_EXCEPTION 0x406d1388
#define DEFAULT_THREAD_STACK_SIZE 1

#pragma pack(push, 8)
struct sThreadNameInfo
{
	DWORD		m_type;            // must be 0x1000
	const char* m_name;      // name
	DWORD		m_threadID;      // -1 for calling thread
	DWORD		m_flags;         // must be 0, reserved for future use
};
#pragma pack(pop)

struct sThreadArgs
{
	const char* m_name;
	ThreadCB	m_cb;
	void*		m_arg;
};

/************************************************************************/
/*                                                                      */
/* STATIC FUNCTIONS						                                */
/*                                                                      */
/************************************************************************/
static DWORD WINAPI ThreadEntryPoint(void* arg)
{
	sThreadArgs* threadArg = (sThreadArgs*)(arg);
	ThreadSetName(threadArg->m_name);

	ThreadCB cb = threadArg->m_cb;
	void* tArg = threadArg->m_arg;

	delete threadArg;

	cb(tArg);
	return 0;
}

/************************************************************************/
/*                                                                      */
/* EXTERNAL FUNCTIONS                                                   */
/*                                                                      */
/************************************************************************/

// create

tThreadHandle ThreadCreate(const char* name, size_t stackSize, ThreadCB cb, void* userData)
{
	sThreadArgs* args = new sThreadArgs();
	args->m_name = name;
	args->m_cb = cb;
	args->m_arg = userData;

	if (stackSize == 0)
	{
		stackSize = DEFAULT_THREAD_STACK_SIZE;
	}

	DWORD id = 0;
	//HANDLE handle = ::CreateThread(NULL, stackSize, ThreadEntryPoint, args, 0, &id);
	tThreadHandle handle = ::CreateThread(NULL, stackSize, ThreadEntryPoint, args, 0, &id);

	//return (tThreadHandle)handle;
	return handle;
}

tThreadHandle ThreadCreate(const char* name, ThreadCB cb, void* arg)
{
	return ThreadCreate(name, 0, cb, arg);
}

tThreadHandle ThreadCreate(ThreadCB cb, void* arg)
{
	return ThreadCreate(nullptr, 0, cb, arg);
}

// release

void ThreadJoin(tThreadHandle handle)
{
	::WaitForSingleObject(handle, INFINITE);
	::CloseHandle(handle);
}


void ThreadJoin(tThreadHandle* handles, const size_t count)
{
	for (size_t i = 0; i < count; ++i)
	{
		ThreadJoin(handles[i]);
	}
}

void ThreadDetach(tThreadHandle handle)
{
	::CloseHandle(handle);
}

void ThreadCreateAndDetach(ThreadCB cb, void* userData)
{
	tThreadHandle handle = ThreadCreate(cb, userData);
	ThreadDetach(handle);
}

void ThreadCreateAndDetach(const char* name, ThreadCB cb, void* userData)
{
	tThreadHandle handle = ThreadCreate(name, cb, userData);
	ThreadDetach(handle);
}

void ThreadCreateAndDetach(const char* name, const size_t stackSize, ThreadCB cb, void* userData)
{
	tThreadHandle handle = ThreadCreate(name, stackSize, cb, userData);
	ThreadDetach(handle);
}

// debug

void ThreadSetName(const char* name)
{
	if (name == nullptr)
	{
		return;
	}

	tThreadID id = ThreadGetCurrentID();

	if (id != 0)
	{
		sThreadNameInfo info;
		info.m_type = 0x1000;
		info.m_name = name;
		info.m_threadID = (DWORD)id;
		info.m_flags = 0;

		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)(&info));
		}
		__except (EXCEPTION_CONTINUE_EXECUTION)
		{

		}
	}
}

tThreadID ThreadGetCurrentID()
{
	return (tThreadID) ::GetCurrentThreadId();
}

tThreadID ThreadGetID(tThreadHandle handle)
{
	return (tThreadID) ::GetThreadId(handle);
}

// control

void ThreadYield()
{
	::SwitchToThread();
}

void ThreadYield(int& current, const int count)
{
	++current;
	if (current >= count)
	{
		ThreadYield();
		current = 0;
	}
}

void ThreadSleep(uint ms)
{
	::Sleep((DWORD)ms);
}


// test

void ThreadTest(void*)
{
	const std::string& path = GetAbsLogPath("garbage", ".dat");
	File* file = new File(path.c_str());
	file->WriteTest();

	delete file;
	file = nullptr;

	DebuggerPrintf("Finished ThreadTest");
}

