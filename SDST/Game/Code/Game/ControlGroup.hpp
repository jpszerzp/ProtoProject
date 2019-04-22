#pragma once

//#include "Engine/Physics/3D/PHYSX/PhysXProcessor.hpp"

#include "Engine/Core/GameObject.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"

#include <vector>

enum eControlID
{
	CONTROL_SPHERE_SPHERE,
	CONTROL_SPHERE_PLANE,
	CONTROL_BOX_PLANE,
	CONTROL_BOX_SPHERE,
	CONTROL_BOX_BOX,
	CONTROL_LINE_LINE,
	CONTROL_HULL_HULL
};

class ControlGroup
{
private:
	std::vector<GameObject*> m_gos;
	Vector3 m_observation_pos;
	eControlID m_id;
	std::vector<Contact3> m_contacts;

	std::vector<Mesh*> m_view;
	float m_textHeight = 0.f;
	Vector2 m_startMin = Vector2::ZERO;

public:
	ControlGroup(GameObject* go1, GameObject* go2, const eControlID& id, const Vector3& observation);
	~ControlGroup(){}

	void RenderCore(Renderer* renderer);
	void RenderUI();

	void ProcessInput();
	void Update(float deltaTime);
	void UpdateDebugDraw();
	void UpdateUI();

	eControlID GetID() const { return m_id; }
	Vector3 GetObservationPos() const { return m_observation_pos; }
};