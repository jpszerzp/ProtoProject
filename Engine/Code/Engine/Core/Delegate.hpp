#pragma once

#include "Engine/Core/ErrorWarningAssert.hpp"

#include <utility>

class Delegate
{
	typedef void* classPtr;
	typedef void (*funcPtr)(classPtr, int);
	typedef std::pair<classPtr, funcPtr> stub;

	// free functions
	template <void (*Function)(int)>
	static __forceinline void FunctionStub(classPtr, int arg_0)
	{
		return (Function)(arg_0);
	}

	// class member functions
	template <class C, void(C::*Function)(int)>
	static __forceinline void ClassMethodStub(classPtr classInstance, int arg_0)
	{
		return (static_cast<C*>(classInstance)->*Function)(arg_0);
	}

public:
	Delegate(void) : m_stub(nullptr, nullptr) {}

	// binds free function
	template <void (*Function)(int)>
	void Bind(void)
	{
		m_stub.first = nullptr;
		m_stub.second = &FunctionStub<Function>;
	}

	// binds class functions
	template <class C, void (C::*Function)(int)>
	void Bind(C* instance)
	{
		m_stub.first = instance;
		m_stub.second = &ClassMethodStub<C, Function>;
	}

	// invokes delegate
	void Invoke(int arg_0) const
	{
		ASSERT_RECOVERABLE(m_stub.second != nullptr, "Cannot invoke unbound delegate");
		return m_stub.second(m_stub.first, arg_0);
	}
	
private:
	stub m_stub;
};