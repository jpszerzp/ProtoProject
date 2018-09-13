#include "Engine/Net/Endianness.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

eEndianness GetPlatformEndianness()
{
	int i = 1;
	char* p = (char*)&i;

	if (p[0] == 1)
	{
		DebuggerPrintf("My machine is little endian\n");
		return LITTLE_ENDIAN;
	}
	else
	{
		DebuggerPrintf("My machine is big endian\n");
		return BIG_ENDIAN;
	}
}

eEndianness GetPlatformEndiannessAnother()
{
	uint32_t sample = 0x01000010;
	if (((uint8_t*)(&sample))[0] == 0x01)
	{
		DebuggerPrintf("My machine is big endian\n");
		return BIG_ENDIAN;
	}
	else 
	{
		DebuggerPrintf("My machine is little endian\n");
		return LITTLE_ENDIAN;
	}
}

void ToBigEndian(const size_t size, const void* data, void* big_endian)
{
	uint8_t* value_ptr = (uint8_t*)data;
	uint8_t* big_endian_ptr = (uint8_t*)big_endian;

	// assuming data is in little endian
	for (int i = 0; i < size; ++i)
		big_endian_ptr[i] = value_ptr[size - 1 - i];
	big_endian_ptr[size] = NULL;
	//big_endian = big_endian_ptr;
}

void ToLittleEndian(const size_t size, const void* data, void* little_endian)
{
	uint8_t* value_ptr = (uint8_t*)data;
	uint8_t* little_endian_ptr = (uint8_t*)little_endian;

	for (int i = 0; i < size; ++i)
		little_endian_ptr[i] = value_ptr[size - 1 - i];
	little_endian_ptr[size] = NULL;
}

void FromBigEndian(const size_t size, const void* data, void* little_from_big_endian)
{
	uint8_t* value_ptr = (uint8_t*)data;
	uint8_t* little_endian_ptr = (uint8_t*)little_from_big_endian;

	for (int i = 0; i < size; ++i)
		little_endian_ptr[i] = value_ptr[size - 1 - i];
	little_endian_ptr[size] = NULL;
}

void FromLittleEndian(const size_t size, const void* data, void* big_from_little_endian)
{
	uint8_t* value_ptr = (uint8_t*)data;
	uint8_t* big_endian_ptr = (uint8_t*)big_from_little_endian;

	for (int i = 0; i < size; ++i)
		big_endian_ptr[i] = value_ptr[size - 1 - i];
	big_endian_ptr[size] = NULL;
}

// Assumes data is in platform endianness, and will convert to supplied endianness;
char* ToEndianness(const size_t size, const void* data, eEndianness endianness)
{
	if (endianness == LITTLE_ENDIAN)
	{
		if (GetPlatformEndianness() != LITTLE_ENDIAN)
		{
			// platform has big endian, data is in big endian, hence big to little endian
			uint8_t* converted = new uint8_t[size];
			ToLittleEndian(size, data, converted);
			return (char*)converted;
		}
	}
	else
	{
		if (GetPlatformEndianness() != BIG_ENDIAN)
		{
			// platform has little endian, data is in little endian, hence little to big endian
			uint8_t* converted = new uint8_t[size];
			ToBigEndian(size, data, converted);
			return (char*)converted;
		}
	}
}

// Assumes data is in supplied endianness, and will convert to platform's endianness
char* FromEndianness(const size_t size, const void* data, eEndianness endianness)
{
	if (endianness == LITTLE_ENDIAN)
	{
		if (GetPlatformEndianness() != LITTLE_ENDIAN)
		{
			// supplied little endian, data is in little endian, but platform is in big endian, hence get big from little endian
			uint8_t* converted = new uint8_t[size];
			FromLittleEndian(size, data, converted);
			return (char*)converted;
		}
	}
	else
	{
		if (GetPlatformEndianness() != BIG_ENDIAN)
		{
			// supplied big endian, data is in big endian, but platform is in little endian, hence get little from big endian
			uint8_t* converted = new uint8_t[size];
			FromBigEndian(size, data, converted);
			return (char*)converted;
		}
	}
}

