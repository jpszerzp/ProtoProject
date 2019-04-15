#pragma once

#include "Engine/Core/Transform.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Physics/3D/RF/CollisionEntity.hpp"

class GameObject
{
public:
	bool		m_physDriven = false;
	bool		m_dead = false;

	bool		m_transparent = false;
	bool		m_drawBorder = false;
	bool		m_drawBasis = false;
	bool		m_debugOn = false;
	bool		m_isInForwardPath = false;
	bool		m_isInBVH = false;
	bool		m_renderableDelayedDelete = false;
	eDepthCompare	m_desiredCompare;
	eCullMode	m_desiredCull;
	eWindOrder	m_desiredOrder;

	// basis mesh
	Mesh* m_forwardBasisMesh = nullptr;
	Mesh* m_upBasisMesh = nullptr;
	Mesh* m_rightBasisMesh = nullptr;

	// Renderable
	Renderable* m_renderable = nullptr;
	Texture* m_direct_tex;

	Entity3* m_physEntity = nullptr;

public:
	virtual void Update(float){}
	virtual void UpdateWithSetPos(const Vector3&){}
	virtual void UpdateInput(float){}
	virtual void UpdateBasis();

	virtual void Render(Renderer* renderer);
	virtual void RenderWithBorder(Renderer* renderer);
    void RenderBasis(Renderer* renderer);

	void ToggleBoundSphereDebugDraw();
	void ToggleBoundBoxDebugDraw();

	virtual void EntityDrivePosition(Vector3 translation);
	virtual void ObjectDrivePosition(Vector3 pos){}

	Vector3 GetWorldPosition();
	Entity3* GetEntity() { return m_physEntity; }
	Vector3 GetPhysicsCenter() const { return m_physEntity->GetEntityCenter(); }		// used when object is physics driven

	GameObject();
	virtual ~GameObject();
};