#pragma once

#include "Engine/Net/Endianness.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include <string>

#define BIT_FLAG(f) (1U << (f))

enum eBytePackerOptionBit : uint
{
	// a byte packer allocated its own memory and 
	// should free it when destroyed; 
	BYTEPACKER_OWNS_MEMORY    = BIT_FLAG(0),

	// If the bytepacker runs out of space, will
	// allocate more memory (maintaining the data that was stored)
	// Must have flag BYTEPACKER_OWNS_MEMORY
	BYTEPACKER_CAN_GROW       = BIT_FLAG(1)
};

typedef uint eBytePackerOptions;

class BytePacker
{
public:
	eEndianness m_endian;
	size_t m_sizeOfHowManyBytes;		// for compression, [11001010b,00000001b] would make this 2
	uint8_t* m_size = new uint8_t[2];	// [11001010b,00000001b]
	uint8_t* m_buffer;
	size_t m_writtenBytes;
	size_t m_readBytes;
	int m_growAcc;
	const static size_t BUFFER_CAPACITY = 1024;

public:
	//BytePacker();
	BytePacker(eEndianness endian = LITTLE_ENDIAN);
	BytePacker(size_t buffer_size, eEndianness endian = LITTLE_ENDIAN);
	BytePacker(size_t buffer_size, void* buffer, eEndianness endian = LITTLE_ENDIAN);
	~BytePacker();

	// sets how much of the buffer is readable
	void SetEndianness(eEndianness endian) { m_endian = endian; }
	bool SetReadableByteCount(size_t byte_count);

	// write data to the end of buffer
	bool WriteBytes(size_t byte_count, const void* data);
	// try to read into out_data, returns how much ended up being read
	size_t ReadBytes(void* out_data, size_t max_byte_count);

	size_t WriteSize(size_t size);			// returns how many bytes used
	uint16_t ReadSize();		// returns how many bytes read, fills out_size

	bool WriteString(const char* str);
	size_t ReadString(char* out_str, size_t max_byte_size);			// max_byte_size enough to contain null terminator as well

	void ResetWrite();
	void ResetRead();

	eEndianness GetEndianness() const { return m_endian; }
	size_t GetWrittenByteCount() const { return m_writtenBytes; }
	size_t GetWritableByteCount() const;
	size_t GetReadableByteCount() const { return m_writtenBytes - m_readBytes; }

	template<typename T>
	void WriteType(T value);

	uint8_t* GetBuffer() { return m_buffer; }

	void Seal();

private:
	
};

//void PackLengthPrint(int length);

// this is to write random types into the buffer
// one application we will use is the type bool to write info about echo
template<typename T>
void BytePacker::WriteType(T value)
{
	// Again, what we know is that we want to pack this into a byte
	// so given the type we can compute its size
	// from there we get how many bytes we expect from T
	size_t byteNum = sizeof(T);
	
	// now we want to align value with uint8_t,
	// because we want to chunk this data in a way it is easy to traverse in either direction,
	// since this time we consider endianness and it is possible to start from backend
	uint8_t* valuePtr = (uint8_t*)(&value);

	WriteBytes(byteNum, valuePtr);
}