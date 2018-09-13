#include "Game/WaterSurface.hpp"
#include "Engine/Renderer/Renderable.hpp"

WaterSurface::WaterSurface()
{
	Renderer* renderer = Renderer::GetInstance();
	m_waterTex = renderer->CreateOrGetTexture("Data/Images/caustic_depth_03.gif");

	//m_surfaceMesh = renderer->CreateOrGetMesh("quad_pcu");
	Vector2 uvBL = Vector2::ZERO;
	Vector2 uvBR = Vector2(128.f, 0.f);
	Vector2 uvTL = Vector2(0.f, 128.f);
	Vector2 uvTR = Vector2(128.f, 128.f);
	m_surfaceMesh = Mesh::CreateQuadTextured(VERT_PCU, uvBL, uvBR, uvTL, uvTR);

	Vector3 pos = Vector3(0.f, 15.f, 0.f);
	Vector3 rot = Vector3(90.f, 0.f, 0.f);
	Vector3 scale = Vector3(256.f, 256.f, 0.f);
	Transform transform = Transform(pos, rot, scale);

	Rgba tint = Rgba::WHITE;
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(nullptr, m_surfaceMesh, transform, tintVec4);
}

WaterSurface::~WaterSurface()
{

}

void WaterSurface::Update(float)
{

}

void WaterSurface::Render(Renderer* renderer)
{
	Shader* shader = renderer->CreateOrGetShader("water");
	renderer->UseShader(shader);

	renderer->SetTexture2D(Renderer::DIFFUSE_MAP_BIND_IDX, m_waterTex);
	renderer->SetSampler2D(Renderer::DIFFUSE_MAP_BIND_IDX, m_waterTex->GetSampler());

	//renderer->m_objectData.model = Matrix44::IDENTITY;
	renderer->m_objectData.model = m_renderable->m_transform.GetWorldMatrix();

	renderer->DrawMesh(m_surfaceMesh);
}
