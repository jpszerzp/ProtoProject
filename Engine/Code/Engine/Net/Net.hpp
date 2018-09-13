#pragma once

#pragma comment(lib, "ws2_32.lib")		// winsock libs

class Net
{
public:
	static bool Startup();
	static void Shutdown();
};
