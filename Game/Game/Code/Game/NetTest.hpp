#pragma once

class NetTest
{
public:
	static void RunNetTest();

private:
	static void EndianTest();
	static void WriteAndReadStringTest();
	static void ReceiveStringTest(uint16_t size_of_encoded, size_t against);
};