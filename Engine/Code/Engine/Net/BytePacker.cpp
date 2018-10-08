#include "Engine/Net/BytePacker.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Util/StringUtils.hpp"

/*
BytePacker::BytePacker()
{
	m_growAcc = 0;
	m_edian = LITTLE_ENDIAN;
	m_buffer = new uint8_t[BUFFER_CAPACITY];
	m_writtenBytes = 0;
	m_readBytes = 0;
}
*/

BytePacker::BytePacker(eEndianness endian)
{
	m_growAcc = 0;
	m_endian = endian;
	m_buffer = new uint8_t[BUFFER_CAPACITY];	// no buffer is given, reserve 1 KB space for the buffer
	m_writtenBytes = 0;							
	m_readBytes = 0;
}

BytePacker::BytePacker(size_t buffer_size, eEndianness endian /*= LITTLE_ENDIAN*/)
{
	m_growAcc = 0;
	m_endian = endian;
	m_buffer = new uint8_t[buffer_size];		// buffer size is given
	m_writtenBytes = 0;
	m_readBytes = 0;
}

BytePacker::BytePacker(size_t buffer_size, void* buffer, eEndianness endian /*= LITTLE_ENDIAN*/)
{
	m_growAcc = 0;
	m_endian = endian;
	m_buffer = (uint8_t*)buffer;		// provided a buffer
	m_writtenBytes = buffer_size;		// buffer_size is size of buffer, assume buffer_size < BUFFER_CAPACITY
	m_readBytes = 0;
}

BytePacker::~BytePacker()
{

}

bool BytePacker::SetReadableByteCount(size_t byte_count)
{
	size_t read_header = m_writtenBytes - byte_count;
	
	if (read_header >= 0)
	{
		m_readBytes = read_header;
		return true;
	}

	return false;
}

bool BytePacker::WriteBytes(size_t byte_count, const void* data)
{
	uint8_t* valuePtr = (uint8_t*)data;
	for (int i = 0; i < byte_count; ++i)
	{
		m_buffer[m_writtenBytes] = valuePtr[i];
		m_writtenBytes++;

		if (m_writtenBytes == ((m_growAcc + 1) * BUFFER_CAPACITY))
		{
			// grow memory
			m_growAcc++;
			uint8_t* newMemory = new uint8_t[(m_growAcc + 1) * BUFFER_CAPACITY];
			
			// copy buffer
			for (size_t j = 0; j < m_writtenBytes; ++j)
				newMemory[j] = m_buffer[j];

			// assign new buffer
			m_buffer = newMemory;
		}
	}

	return true;
}

size_t BytePacker::ReadBytes(void* out_data, size_t max_byte_count)
{
	size_t length = (m_growAcc + 1) * BUFFER_CAPACITY;
	
	uint8_t* read_data = new uint8_t[max_byte_count];
	size_t read_amount = 0;
	for (int i = 0; i < max_byte_count; ++i)
	{
		read_data[i] = m_buffer[m_readBytes];
		m_readBytes++;
		read_amount++;

		if (m_readBytes == length)
		{
			// reaching end of buffer, stop reading 
			break;
		}
	}

	out_data = read_data;
	return read_amount;
}

// WriteSize(202) would return 2, and fill m_size with [11001010b,00000001b]
size_t BytePacker::WriteSize(size_t size)
{
	size_t byteCount = 0;
	// since we use size_t, we assume 64 bits (8 bytes)
	// we start by chunking 7 bits on the right (right shift)
	// if the remained bits is not equal to 0, we put a 1 on high bit, and continue chunking
	// if the remained bits is equivalent to 0, we put a 0 on high bit and quit
	// in both cases, once we put a 0 or 1 at the high bit, we increment byte count

	//size_t size_hold = size;
	//std::vector<uint8_t> remained;
	//RightShiftRemainedBit(remained, &size_hold, 7);
	//bool continuing = false;
	//for each (uint8_t i in remained)
	//{
	//	if (i != 0)
	//		continuing = true;
	//}

	while ((size>>7) > 0)
	{
		std::vector<uint8_t> lostBits;
		RightShiftLostBit(lostBits, &size, 7);
		SetHighBit(lostBits, 1);

		// now the lostBits contains the binary of the packed result we want
		// since this is a byte packer, we know the result has size of one byte, hence we can use type uint8_t
		uint8_t packedSize = (uint8_t)(DecimalFromBinary(lostBits));

		// now we get the packed byte, we need to push it to buffer
		m_size[byteCount] = packedSize;

		byteCount++;

		//size = (size >> 7);
	}

	// the lost bit is of 7 bits
	std::vector<uint8_t> lostBits;
	RightShiftLostBit(lostBits, &size, 7);
	// putting 0 at high bit make this packed result a full byte, so it is still safe to store with uint8_t
	SetHighBit(lostBits, 0);
	uint8_t packedSize = (uint8_t)(DecimalFromBinary(lostBits));
	// for the final packed byte, we still need to push it to buffer ofc
	m_size[byteCount] = packedSize;

	m_sizeOfHowManyBytes = byteCount + 1;

	return m_sizeOfHowManyBytes;
}

// ReadSize(out_size) would return 2 since size info is stored with 2 bytes, 
// and fill out_size with 202 given m_size to be [11001010b,00000001b]
uint16_t BytePacker::ReadSize()
{
	std::vector<uint8_t> res = BinaryFromDecimal(m_size[m_sizeOfHowManyBytes - 1]);

	for (int i = (int)m_sizeOfHowManyBytes - 1; i >= 0; --i)
	{
		int last_idx = i - 1;
		if (last_idx >= 0)
		{
			uint8_t last_enc = m_size[last_idx];
			std::vector<uint8_t> last_bin = BinaryFromDecimal(last_enc);	// length of 8
			for (int j = 0; j < 7; ++j)
				res.push_back(0);
			std::vector<uint8_t>::iterator it;
			it = res.begin();
			res.insert(it, 0);			// fill a zero at head, now it has 8-times bits
										// vector ready to OR

			size_t binary_size = res.size();
			int l = 0;
			for (int k = 7; k >= 0; --k)
			{
				if ( k != 0 )
				{
					int last_bit = last_bin[k];

					int idx = (int)binary_size - 1 - l;
					l++;
					int this_bit = res[idx];

					int new_bit = last_bit | this_bit;

					// alter the bit
					res[idx] = (uint8_t)new_bit;
				}
			}

			// at this point bin is updated with new bits
			// see if the new bin starts with a byte of zero after updated
			// if yes, abort the loop
			// if no, continue the loop
			bool continuing = false;
			for (int m = 7; m >= 0; --m)
			{
				int the_bit = res[m];
				if (the_bit == 1)
				{
					continuing = true;
					break;
				}
			}

			if (!continuing)
				break;
		}
		else
		{
			size_t res_size = m_size[m_sizeOfHowManyBytes - 1];
			return (uint16_t)res_size;
		}
	}

	// now the bin vector has a byte of zero at its head, chunk it
	std::vector<uint8_t>::const_iterator first = res.begin() + 8;
	std::vector<uint8_t>::const_iterator last = res.end();
	std::vector<uint8_t> finalBits(first, last);
	size_t decoded_size = DecimalFromBinary(finalBits);

	return (uint16_t)decoded_size;		
}


bool BytePacker::WriteString(const char* str)
{
	size_t stringSize = strlen(str);
	return WriteBytes(stringSize, str);
}

size_t BytePacker::ReadString(char* out_str, size_t max_byte_size)
{
	return ReadBytes(out_str, max_byte_size);
}

void BytePacker::ResetWrite()
{
	m_writtenBytes = 0;
}

void BytePacker::ResetRead()
{
	m_readBytes = 0;
}

size_t BytePacker::GetWritableByteCount() const
{
	size_t length = (m_growAcc + 1) * BUFFER_CAPACITY;
	return (length - m_writtenBytes);
}

void BytePacker::Seal()
{
	m_buffer[m_writtenBytes] = NULL;	// do not increment write head
}
