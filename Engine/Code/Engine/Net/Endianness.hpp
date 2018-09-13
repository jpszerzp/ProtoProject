#pragma once

#include <stdint.h>

enum eEndianness 
{
	BIG_ENDIAN = 0,
	LITTLE_ENDIAN,
	NUM_ENDIAN
};

eEndianness GetPlatformEndianness();
eEndianness GetPlatformEndiannessAnother();
void ToBigEndian(const size_t size, const void* data, void* to_big_endian);	// assume data in platform endianness, convert to big endian
void ToLittleEndian(const size_t size, const void* data, void* to_little_endian); // assume data in platform endianness, convert to little endian
void FromBigEndian(const size_t size, const void* data, void* little_from_big_endian);	// assume data in big endian, convert to platform endianness
void FromLittleEndian(const size_t size, const void* data, void* big_from_little_endian); // assume data in little endian, convert to platform endianness
char* ToEndianness(const size_t size, const void* data, eEndianness endianness);
char* FromEndianness(const size_t size, const void* data, eEndianness endianness);