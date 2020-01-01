#pragma once

#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void FreeFunc(int value);

class DelegateTest
{
public:
	void MemberFunc(int value)
	{
		// delegated member function
		std::string arg = Stringf("Member delegation invoke test: %i\n", value);
		DebuggerPrintf(arg.c_str());
	}
};

void RunDelegateTest();