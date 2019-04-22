//#include "Engine/Physics/3D/PHYSX/PhysXProcessor.hpp"

#include "Engine/Net/Endianness.hpp"
#include "Engine/Net/BytePacker.hpp"
#include "Engine/Net/RemoteCommandService.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/NetTest.hpp"

void NetTest::RunNetTest()
{
	EndianTest();
	WriteAndReadStringTest();
}

void NetTest::EndianTest()
{
	eEndianness e1 = GetPlatformEndianness();
	eEndianness e2 = GetPlatformEndiannessAnother();
	bool endianness_consistent = e1 == e2;
	ASSERT_OR_DIE(endianness_consistent, "Endianness check is not consistent");
	DebuggerPrintf("Endianness check is consistent, see the output panel to tell the endianness\n");
}

void NetTest::WriteAndReadStringTest()
{
	////////////////////////////// THE SEND //////////////////////////////
	BytePacker msg(BIG_ENDIAN);
	msg.WriteType<uint8_t>(1);			// true
	msg.WriteString("hell5yeah11this1");
	const size_t len = msg.GetWrittenByteCount();
	ASSERT_OR_DIE(len == 17, "Number of bytes written is wrong");
	DebuggerPrintf("Length of write string is correct\n");

	// endianness of write string
	uint16_t uslen = (uint16_t)len;		// 17
	char* to_big_endian = new char[1024];				
	const uint8_t* buff = msg.GetBuffer();
	to_big_endian = ToEndianness(uslen, buff, msg.m_endian);
	to_big_endian[uslen] = NULL;
	DebuggerPrintf(to_big_endian);

	// write size test: 17
	size_t encoded_byte_num_17 = msg.WriteSize(uslen);		// writing size of 17
	ASSERT_OR_DIE(encoded_byte_num_17 == 1, "Number of encoded bytes for 17 is wrong");
	DebuggerPrintf("\nNumber of encoded bytes for 17 is correct\n");

	std::vector<uint8_t> encoded_binary;
	for (int i = 0; i < encoded_byte_num_17; ++i)
	{
		uint8_t encoded_byte = msg.m_size[i];
		std::vector<uint8_t> binary = BinaryFromDecimal(encoded_byte);

		for (int j = 0; j < binary.size(); ++j)			// size of 8
		{
			encoded_binary.push_back(binary[j]);
		}
	}
	uint16_t encoded_size_two_bytes_17 = (uint16_t)DecimalFromBinary(encoded_binary);
	ASSERT_OR_DIE(encoded_binary.size() == 8, "When writing size 17, size of encoded binary is not right");
	DebuggerPrintf("When writing size 17, size of encoded binary is right\n");
	ASSERT_OR_DIE(encoded_binary[0] == 0, "When writing size 17, high 0 bit does not match");
	DebuggerPrintf("When writing size 17, high 0 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[1] == 0, "When writing size 17, high 1 bit does not match");
	DebuggerPrintf("When writing size 17, high 1 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[2] == 0, "When writing size 17, high 2 bit does not match");
	DebuggerPrintf("When writing size 17, high 2 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[3] == 1, "When writing size 17, high 3 bit does not match");
	DebuggerPrintf("When writing size 17, high 3 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[4] == 0, "When writing size 17, high 4 bit does not match");
	DebuggerPrintf("When writing size 17, high 4 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[5] == 0, "When writing size 17, high 5 bit does not match");
	DebuggerPrintf("When writing size 17, high 5 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[6] == 0, "When writing size 17, high 6 bit does not match");
	DebuggerPrintf("When writing size 17, high 6 bit does match\n");
	ASSERT_OR_DIE(encoded_binary[7] == 1, "When writing size 17, high 7 bit does not match");
	DebuggerPrintf("When writing size 17, high 7 bit does match\n");
	encoded_binary.clear();

	// 202
	BytePacker msg1(BIG_ENDIAN);
	size_t encoded_byte_num_202 = msg1.WriteSize(202);
	for (int i = 0; i < encoded_byte_num_202; ++i)
	{
		uint8_t encoded_byte = msg1.m_size[i];
		std::vector<uint8_t> binary = BinaryFromDecimal(encoded_byte);

		for (int j = 0; j < binary.size(); ++j)			// size of 8
			encoded_binary.push_back(binary[j]);
	}
	uint16_t encoded_size_two_bytes_202 = (uint16_t)DecimalFromBinary(encoded_binary);
	ASSERT_OR_DIE(msg1.m_sizeOfHowManyBytes == 2, "Number of encoded bytes for 202 is wrong");
	DebuggerPrintf("Number of encoded bytes for 202 is correct\n");

	// 8675309(65535)
	BytePacker msg2(BIG_ENDIAN);
	msg2.WriteSize(8675309);				// chunked to 65535
	ASSERT_OR_DIE(msg2.m_sizeOfHowManyBytes == 4, "Number of encoded bytes for 8675309(65535) is wrong");
	DebuggerPrintf("Number of encoded bytes for 8675309(65535) is correct\n");

	// null terminate buffers
	BytePacker msg3(BIG_ENDIAN);
	msg3.WriteType<uint8_t>(2);				// false
	msg3.WriteString("testing echo false");
	msg3.Seal();
	const size_t len_3 = msg3.GetWrittenByteCount();
	uint16_t uslen_3 = (uint16_t)len_3;		// 17
	char* to_big_endian_3 = new char[1024];				
	uint8_t* buff_3 = msg3.GetBuffer();
	// test: before flipping, null the buffer
	to_big_endian_3 = ToEndianness(uslen_3, buff_3, msg3.m_endian);

	DebuggerPrintf("buffer before flip: %s", buff_3);
	DebuggerPrintf("\nbuffer after flip: %s\n", to_big_endian_3);

	// help: size of 5
	BytePacker msg4(BIG_ENDIAN);
	msg4.WriteType<uint8_t>(1);
	msg4.WriteString("help");
	msg4.Seal();
	const size_t len_4 = msg4.GetWrittenByteCount();
	uint16_t uslen_4 = (uint16_t)len_4;
	char* to_big_endian_4 = new char[1024];
	uint8_t* buff_4 = msg4.GetBuffer();
	to_big_endian_4 = ToEndianness(uslen_4, buff_4, msg4.m_endian);
	size_t encoded_byte_num_4 = msg4.WriteSize(uslen_4);
	DebuggerPrintf("buffer before flip: %s\n", buff_4);
	DebuggerPrintf("buffer after flip: %s", to_big_endian_4);
	ASSERT_OR_DIE(encoded_byte_num_4 == 1, "help: encoded byte number correct");
	for (int i = 0; i < encoded_byte_num_4; ++i)
	{
		uint8_t encoded_byte = msg4.m_size[i];
		std::vector<uint8_t> binary = BinaryFromDecimal(encoded_byte);

		for (int j = 0; j < binary.size(); ++j)			// size of 8
		{
			encoded_binary.push_back(binary[j]);
		}
	}
	uint16_t encoded_size_two_bytes = (uint16_t)DecimalFromBinary(encoded_binary);

	////////////////////////////// THE RECEIVE //////////////////////////////
	ReceiveStringTest(encoded_size_two_bytes, 5);
	ReceiveStringTest(encoded_size_two_bytes_17, 17);
	ReceiveStringTest(encoded_size_two_bytes_202, 202);
}

void NetTest::ReceiveStringTest(uint16_t size_of_encoded, size_t against)
{
	BytePacker unpacker = BytePacker(BIG_ENDIAN);
	unpacker.m_sizeOfHowManyBytes = 0;
	std::vector<uint8_t> binaries = BinaryFromDecimal16(size_of_encoded);
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
	ASSERT_OR_DIE(true_size == against, "Outsize of reading size is not correct " + std::to_string(against));
}

