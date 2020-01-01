#include "Game/Test/DelegateTest.hpp"
#include "Engine/Core/Delegate.hpp"

void FreeFunc(int value)
{
	// delegated free function
	std::string arg = Stringf("\nGlobal delegation invoke test: %i\n", value);
	DebuggerPrintf(arg.c_str());
}

void RunDelegateTest()
{
	Delegate delegation;
	delegation.Bind<&FreeFunc>();
	delegation.Invoke(10);

	DelegateTest testClass;
	delegation.Bind<DelegateTest, &DelegateTest::MemberFunc>(&testClass);
	delegation.Invoke(20);
}
