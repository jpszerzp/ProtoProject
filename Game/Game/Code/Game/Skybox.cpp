#include "Game/Skybox.hpp"

Skybox::Skybox()
{
	Renderer* renderer = Renderer::GetInstance();
	m_skybox = new TextureCube();
	m_skybox->FromImage("Data/Images/skybox.jpg");
	m_boxMesh = renderer->CreateOrGetMesh("cube_pcu");
}


Skybox::~Skybox()
{
	delete m_skybox;
	m_skybox = nullptr;
}


void Skybox::Update(float)
{

}


void Skybox::Render(Renderer* renderer)
{
	Shader* shader = renderer->CreateOrGetShader("skybox");
	renderer->UseShader(shader);

	renderer->SetTextureCube2D(Renderer::SKYBOX_TEX_BIND_IDX, m_skybox);
	renderer->SetSampler2D(Renderer::SKYBOX_TEX_BIND_IDX, m_skybox->m_sampler);

	renderer->m_objectData.model = Matrix44::IDENTITY;

	renderer->DrawMesh(m_boxMesh);
}