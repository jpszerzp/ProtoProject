#include "Engine/Renderer/MaterialProperty.hpp"
#include "Engine/Renderer/GLFunctions.hpp"

sPropertyInfo::sPropertyInfo()
{

}


sPropertyInfo::~sPropertyInfo()
{
	// no need to delete owning block info as that destructor gets called first
	//m_owningBlock = nullptr;
}


PropertyVec4::PropertyVec4(const char* name, const Vector4& value)
{
	m_name = name;
	m_value = value;
}


void PropertyVec4::Bind(uint handle)
{
	GLuint idx = glGetUniformLocation(handle, m_name.c_str());
	if (idx >= 0)
	{
		glUniform4fv(idx, 1, (GLfloat*)&m_value);
	}
}