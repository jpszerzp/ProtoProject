#include "Engine/Core/HullObject.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Renderable.hpp"
#include "Engine/Input/InputSystem.hpp"

#include <assert.h>

void HullObject::Render(Renderer* renderer)
{
	TODO("Actually render model if there is one");
	if (m_renderable->m_mesh != nullptr)
	{

	}

	RenderBasis(renderer);
	
	// render the hull, and maybe its basis
	m_hull->RenderHull(renderer);
	m_hull->RenderBasis(renderer);
}

void HullObject::Update(float)
{
	InputSystem* input = InputSystem::GetInstance();
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		m_drawBasis = !m_drawBasis;
		m_hull->m_drawBasis = !m_hull->m_drawBasis;
	}

	UpdateBasis();

	m_hull->UpdateHull();
	m_hull->UpdateBasis();
}

HullObject::HullObject(const Vector3& pos, const Vector3& ext, const Vector3& rot, const Vector3& scale, const Rgba& tint,
	std::string matName, eMoveStatus moveStat, eBodyIdentity bid, int sampleCount, std::string fileName /*= ""*/, 
	bool multipass /*= false*/, eCompare compare /*= COMPARE_LESS*/, eCullMode cull /*= CULLMODE_BACK*/, eWindOrder wind /*= WIND_COUNTER_CLOCKWISE*/)
{
	Renderer* r = Renderer::GetInstance();

	// Set renderable
	Material* material;
	if (!multipass)
		material = r->CreateOrGetMaterial(matName);
	else
		material = r->CreateOrGetStagedMaterial(matName);

	// based on model file, prepare for render
	Mesh* mesh = nullptr;
	if (!fileName.empty())
		mesh = r->CreateOrGetMesh(fileName);

	// The way we set transform of hull is that we assume it has 0 rotation and unit scale (line 877 of QuickHull.cpp)
	// therefore when we change object transform here with rot and scale, we also make sure that same data is passed 
	// to Quickhull somewhere. In this implementation, we set this game object as a ref in QH instance, hence
	// retrieving rot and scale data from this ref
	Transform transform = Transform(pos, rot, scale);

	// Set tint
	Vector4 tintVec4;
	tint.GetAsFloats(tintVec4.x, tintVec4.y, tintVec4.z, tintVec4.w);

	m_renderable = new Renderable(material, mesh, transform, tintVec4);

	m_desiredCompare = compare;
	m_desiredCull = cull;
	m_desiredOrder = wind;

	// set up hull
	if (fileName.empty())
	{
		// if there is no model to import, we use center and half extension to generate point randomly
		Vector3 qhMin = pos - ext;
		Vector3 qhMax = pos + ext;
		m_hull = new QuickHull(sampleCount, qhMin, qhMax, true);
	}
	else
	{
		TODO("If there is a model to import, the way QH generate point needs to respect that model; may need a different constructor for QH");
	}

	assert(m_hull != nullptr);
}

HullObject::~HullObject()
{
	delete m_renderable;
	m_renderable = nullptr;
}
