#pragma once

#include "Engine/Core/GameObject.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"

#include <vector>

enum eControlID
{
	CONTROL_SPHERE_SPHERE
};

class ControlGroup
{

private:
	std::vector<GameObject*> m_gos;
	Vector3 m_observation_pos;
	eControlID m_id;
	Contact3 m_contact;
	bool m_intersect;

public:
	ControlGroup(GameObject* go1, GameObject* go2, eControlID id);
	~ControlGroup(){}

	void ProcessInput();
	void Render(Renderer* renderer);
	void Update(float deltaTime);

	const eControlID& GetID() const { return m_id; }
	bool IsIntersect() const { return m_intersect; }
	std::string GetControlIDString() const;
	std::string GetPointString() const;
	std::string GetNormalString() const;
	std::string GetPenetrationString() const;
};