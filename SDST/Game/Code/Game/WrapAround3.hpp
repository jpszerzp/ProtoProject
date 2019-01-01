#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"

#include <vector>

class WrapAround
{
public:
	AABB3 m_bounds;
	std::vector<GameObject*> m_gos;
	Mesh* m_mesh = nullptr;
	Transform m_transform;
	Vector3 m_positions[8];
	int m_pos_idx = 0;
	
public:
	WrapAround(const Vector3& min, const Vector3& max, 
		const Vector3& p1 = Vector3::ZERO, const Vector3& p2 = Vector3::ZERO, 
		const Vector3& p3 = Vector3::ZERO, const Vector3& p4 = Vector3::ZERO,
		const Vector3& p5 = Vector3::ZERO, const Vector3& p6 = Vector3::ZERO,
		const Vector3& p7 = Vector3::ZERO, const Vector3& p8 = Vector3::ZERO);
	~WrapAround();

	void Update();

	void Render(Renderer* renderer);
};