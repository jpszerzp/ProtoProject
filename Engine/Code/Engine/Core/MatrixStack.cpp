#include "Engine/Core/MatrixStack.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


MatrixStack::MatrixStack()
{
	UNIMPLEMENTED();
}


void MatrixStack::Push()
{
	UNIMPLEMENTED();

}


void MatrixStack::Pop()
{
	UNIMPLEMENTED();

}


const Matrix44& MatrixStack::GetTop()
{
	return m_stack.top();
}


void MatrixStack::Load( Matrix44 const & )
{
	UNIMPLEMENTED();

}


void MatrixStack::LoadIdentity()
{
	UNIMPLEMENTED();

}


void MatrixStack::Transform( Matrix44 const & )
{
	UNIMPLEMENTED();

}
