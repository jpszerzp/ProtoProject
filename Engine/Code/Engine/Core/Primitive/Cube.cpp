#include "Engine/Core/Primitive/Cube.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Physics/3D/CubeEntity3.hpp"

Cube::Cube()
{

}


Cube::Cube(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint,
	std::string meshName, std::string materialName, 
	eMoveStatus moveStat, eBodyIdentity bid, bool multipass,
	eDepthCompare compare, eCullMode cull, eWindOrder order, bool drawBorder)
{
	Renderer* renderer = Renderer::GetInstance();

	// Set renderable
	Material* material;
	if (!multipass)
		material = renderer->CreateOrGetMaterial(materialName);
	else
		material = renderer->CreateOrGetStagedMaterial(materialName);

	Mesh* mesh = renderer->CreateOrGetMesh(meshName);
	Transform transform = Transform(pos, rot, scale);

	// Set tint
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(material, mesh, transform, tintVec4);

	m_desiredCompare = compare;
	m_desiredCull = cull;
	m_desiredOrder = order;

	Vector3 halfAABB = scale / 2.f;
	Vector3 aabbMin = pos - halfAABB;
	Vector3 aabbMax = pos + halfAABB;
	AABB3 aabb3 = AABB3(aabbMin, aabbMax);
	if (bid == BODY_PARTICLE)
		m_physEntity = new CubeEntity3(aabb3, moveStat);
	TODO("cube rigid body");
	m_physEntity->SetEntityForPrimitive();

	// later for stencil test
	m_drawBorder = drawBorder;
}


Cube::Cube(const Vector3& pos, const Vector3& rot, const Vector3& scale, const Rgba& tint, 
	std::string fp, std::string sp, bool drawBorder /*= false*/)
{
	Renderer* renderer = Renderer::GetInstance();

	Shader* shader = renderer->MakeShader(sp);
	Mesh* mesh = renderer->CreateOrGetMesh(fp);
	Transform transform = Transform(pos, rot, scale);
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(shader, mesh, transform, tintVec4);

	m_drawBorder = drawBorder;
}

Cube::~Cube()
{
	delete m_renderable;
	m_renderable = nullptr;
}


void Cube::Update(float deltaTime)
{
	if (m_physDriven)
	{
		m_physEntity->Integrate(deltaTime);
		m_physEntity->UpdateTransforms();
		m_physEntity->UpdatePrimitives();

		m_renderable->m_transform = m_physEntity->GetEntityTransform();
	}

	UpdateBasis();
}

void Cube::Render(Renderer* renderer)
{
	if (!m_drawBorder)
	{
		Mesh* mesh = m_renderable->m_mesh;
		const Transform& transform = m_renderable->m_transform;

		if (mesh != nullptr)
		{
			Shader* shader = m_renderable->GetShader();
			renderer->UseShader(shader);

			// texture
			Texture* tex = renderer->CreateOrGetTexture("Data/Images/couch/couch_diffuse.png");
			renderer->SetTexture2D(0, tex);
			renderer->SetSampler2D(0, tex->GetSampler());

			// color
			renderer->m_colorData.rgba = m_renderable->GetTint();

			// model
			renderer->m_objectData.model = transform.GetWorldMatrix();

			// camera data is set thru draw

			renderer->Draw(mesh);
		}
	}
	else
		RenderWithBorder(renderer);
}