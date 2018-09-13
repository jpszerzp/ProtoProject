
#define WIN32_LEAN_AND_MEAN

#include "Engine/Net/Net.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/WindowsCommon.hpp"

bool Net::Startup()
{
	// pick version
	WORD version = MAKEWORD(2,2);

	WSADATA data;
	int32_t error = WSAStartup(version, &data);

	ASSERT_OR_DIE(error == 0, "Cannot start up net work");
	return (error == 0);
}

void Net::Shutdown()
{
	::WSACleanup();
}