#include "Engine/Renderer/Drawcall.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


Drawcall::Drawcall()
{
	for (int idx = 0; idx < MAX_LIGHTS; ++idx)
	{
		m_lightIndices[idx] = -1;
	}
}


Drawcall::~Drawcall()
{

}


int Drawcall::GetLayer()
{
	Shader* shader = m_material->m_shader;
	ShaderChannel* channel = m_material->m_channel;

	if (shader != nullptr)
	{
		return shader->m_layer;
	}
	else if (channel != nullptr)
	{
		return channel->m_layer;
	}
	else
	{
		ASSERT_OR_DIE(((shader != nullptr) || (channel != nullptr)), "both shader and channel are null pointers!");
		return -1;
	}
}


eRenderQueue Drawcall::GetQueue()
{
	Shader* shader = m_material->m_shader;
	ShaderChannel* channel = m_material->m_channel;

	if (shader != nullptr)
	{
		return shader->m_queue;
	}
	else if (channel != nullptr)
	{
		return channel->m_queue;
	}
	else
	{
		ASSERT_OR_DIE(((shader != nullptr) || (channel != nullptr)), "both shader and channel are null pointers!");
		return NUM_OF_RENDER_QUEUE;
	}
}