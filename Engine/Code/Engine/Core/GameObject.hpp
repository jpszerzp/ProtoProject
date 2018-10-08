#pragma once

#include "Engine/Core/Transform.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Physics/3D/Entity3.hpp"

class GameObject
{
public:
	bool		m_physDriven = false;
	bool		m_dead = false;

	bool		m_drawBasis = false;
	bool		m_debugOn = false;
	bool		m_isInForwardPath = false;
	bool		m_renderableDelayedDelete = false;
	eCompare	m_desiredCompare;
	eCullMode	m_desiredCull;
	eWindOrder	m_desiredOrder;

	// basis mesh
	Mesh* m_forwardBasisMesh = nullptr;
	Mesh* m_upBasisMesh = nullptr;
	Mesh* m_rightBasisMesh = nullptr;

	// Renderable
	Renderable* m_renderable = nullptr;

	Entity3* m_physEntity = nullptr;

public:
	virtual void Update(float){}
	virtual void UpdateInput(float){}
	virtual void UpdateBasis();

	virtual void Render(Renderer* renderer);
    void RenderBasis(Renderer* renderer);

	void ToggleBoundSphereDebugDraw();
	void ToggleBoundBoxDebugDraw();

	virtual void EntityDrivePosition(Vector3 translation);
	virtual void ObjectDrivePosition(Vector3 pos){}

	Vector3 GetWorldPosition();
	Entity3* GetEntity() { return m_physEntity; }

	GameObject();
	virtual ~GameObject();
};