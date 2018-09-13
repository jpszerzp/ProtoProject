#pragma once
#include "Engine/Core/EngineCommon.hpp"

// handle
//struct tThreadHandleType;
//typedef tThreadHandleType* tThreadHandle;
typedef void* tThreadHandle;
typedef uint tThreadID;
typedef void (*ThreadCB) (void* userData);

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/

// creating
tThreadHandle ThreadCreate(ThreadCB cb, void* userData = nullptr);
tThreadHandle ThreadCreate(const char* name, ThreadCB cb, void* userData = nullptr);
tThreadHandle ThreadCreate(const char* name, size_t stackSize, ThreadCB cb, void* userData = nullptr);

// join and detach - releasing
void ThreadJoin(tThreadHandle th);
void ThreadJoin(tThreadHandle* handles, const size_t count);
void ThreadDetach(tThreadHandle th);
void ThreadCreateAndDetach(ThreadCB cb, void* userData = nullptr);
void ThreadCreateAndDetach(const char* name, ThreadCB cb, void* userData = nullptr);
void ThreadCreateAndDetach(const char* name, const size_t stackSize, ThreadCB cb, void* userData = nullptr);

// control
void ThreadSleep(uint ms);
void ThreadYield();
void ThreadYield(int& current, const int count);

// debug
void ThreadSetName(const char* name);
tThreadID ThreadGetCurrentID();
tThreadID ThreadGetID(tThreadHandle handle);

// later
void ThreadSetCoreAffinity( uint core ); 
void ThreadSetPriority( uint priority ); 

// test
void ThreadTest(void*);

class Thread
{

};