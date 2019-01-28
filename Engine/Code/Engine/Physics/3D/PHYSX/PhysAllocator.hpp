#pragma once

#include "PxPhysicsAPI.h"
using namespace physx;

class PhysAllocator : public PxAllocatorCallback
{
public:
	void* allocate(size_t size, const char*, const char*, int)
	{
		void* ptr = ::malloc(size);
		ASSERT_OR_DIE((reinterpret_cast<size_t>(ptr) & 15) == 0, "False physx allocation");

		// return allocation
		return ptr;
	}

	void deallocate(void* ptr)
	{
		// free allocation
		::free(ptr);
	}
};

