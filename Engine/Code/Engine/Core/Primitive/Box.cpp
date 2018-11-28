#include "Engine/Core/Primitive/Box.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Physics/3D/BoxRB3.hpp"
#include "Engine/Physics/3D/BoxEntity3.hpp"

Box::Box()
{

}


Box::Box(Vector3 pos, Vector3 rot, Vector3 scale, Rgba tint, std::string meshName, std::string matName, eMoveStatus moveStat, eBodyIdentity bid, bool multipass /*= false*/, eCompare compare /*= COMPARE_LESS*/, eCullMode cull /*= CULLMODE_BACK*/, eWindOrder order /*= WIND_COUNTER_CLOCKWISE*/)
{
	Renderer* renderer = Renderer::GetInstance();

	// Set renderable
	Material* material;
	if (!multipass)
		material = renderer->CreateOrGetMaterial(matName);
	else
		material = renderer->CreateOrGetStagedMaterial(matName);

	Mesh* mesh = renderer->CreateOrGetMesh(meshName);
	Transform transform = Transform(pos, rot, scale);

	// Set tint
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(material, mesh, transform, tintVec4);

	m_desiredCompare = compare;
	m_desiredCull = cull;
	m_desiredOrder = order;

	Vector3 forward = transform.GetWorldForward().GetNormalized();
	Vector3 up = transform.GetWorldUp().GetNormalized();
	Vector3 right = transform.GetWorldRight().GetNormalized();
	OBB3 obb3 = OBB3(pos, forward, up, right, scale / 2.f);
	if (bid == BODY_RIGID)
		m_physEntity = new BoxRB3(5.f, obb3, rot, moveStat);
	else if (bid == BODY_PARTICLE)
		m_physEntity = new BoxEntity3(obb3, rot, moveStat);
	m_physEntity->SetEntityForPrimitive();
}

Box::~Box()
{
	delete m_renderable;
	m_renderable = nullptr;
}

void Box::Update(float deltaTime)
{
	if (m_physDriven)
	{
		if (m_physEntity->GetEntityMoveStatus() != MOVE_STATIC)
		{
			m_physEntity->Integrate(deltaTime);
			m_physEntity->UpdateTransforms();
			m_physEntity->UpdatePrimitives();

			m_renderable->m_transform = m_physEntity->GetEntityTransform();
		}
	}

	UpdateBasis();
}