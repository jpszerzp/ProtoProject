#pragma once

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Util/StringUtils.hpp"

#include "PxPhysicsAPI.h"
using namespace physx;

class PhysErrorCallback : public PxErrorCallback
{
public:
	PhysErrorCallback() {}
	~PhysErrorCallback() {}

	virtual void reportError(PxErrorCode::Enum code, const char* msg, const char* file, int line)
	{
		// error processing code...

		// code
		int err_code = (int)(code);

		// integrated msg
		std::string err_str = Stringf("Error: code %i with msg %s in file %s at line %i", err_code, msg, file, line);

		DebuggerPrintf(err_str.c_str());
	}
};