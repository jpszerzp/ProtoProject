#include "Engine/Core/Light/Light.hpp"
#include "Engine/Renderer/Renderable.hpp"


Light::Light(Vector3 pos, Vector3 rot, Vector3 scale,
	Rgba color, Vector3 attenuation, Vector3 specAttenuation, Vector3 direction,
	float innerAngle, float outerAngle, float directionFac)
{
	Renderer* renderer = Renderer::GetInstance();

	Vector4 frameColorVec4;
	m_lightColor = color;
	color.GetAsFloats(frameColorVec4.x, frameColorVec4.y, frameColorVec4.z, frameColorVec4.w);

	Material* material = renderer->CreateOrGetMaterial("");
	material->m_shader = renderer->CreateOrGetShader("wireframe");
	material->FillPropertyBlocks();
	material->FillTextures();
	Mesh* mesh = renderer->CreateOrGetMesh("sphere_pcu");
	Transform transform = Transform(pos, rot, scale);

	// visual renderable
	m_renderable = new Renderable(material, mesh, transform, frameColorVec4);

	m_attenuation = attenuation;
	m_specAttenuation = specAttenuation;
	m_direction = direction;
	m_innerAngle = innerAngle;
	m_outerAngle = outerAngle;
	m_directionFac = directionFac;

	m_mat_amb = Vector3(.2f);
	m_mat_diff = Vector3(.5f);
	m_mat_spec = Vector3::ONE;
}


Light::~Light()
{

}