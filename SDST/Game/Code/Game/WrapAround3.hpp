#pragma once

//#include "Engine/Physics/3D/PHYSX/PhysXProcessor.hpp"

#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Physics/3D/BVH3.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
//#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"

#include <vector>

class Physics3State;

class WrapAround
{
public:
	// toggle, if this wraparound is for particles
	bool m_particle = false;

	// render
	AABB3 m_bounds;
	Mesh* m_mesh = nullptr;
	Transform m_transform;
	Vector3 m_positions[8];
	int m_pos_idx = 0;

	// storage
	std::vector<CollisionPrimitive*> m_primitives;
	//std::vector<PhysXObject*> m_phys_obj;

	// state reference
	Physics3State* m_physState = nullptr;
	
public:
	WrapAround(const Vector3& min, const Vector3& max, 
		const Vector3& p1 = Vector3::ZERO, const Vector3& p2 = Vector3::ZERO, 
		const Vector3& p3 = Vector3::ZERO, const Vector3& p4 = Vector3::ZERO,
		const Vector3& p5 = Vector3::ZERO, const Vector3& p6 = Vector3::ZERO,
		const Vector3& p7 = Vector3::ZERO, const Vector3& p8 = Vector3::ZERO);
	~WrapAround();

	void Update();
	void UpdateWraparound();

	void Render(Renderer* renderer);
	void RenderWraparounds(Renderer* renderer);

	void RemovePrimitive(CollisionPrimitive* prim);
	//void RemovePhysXObj(PhysXObject* px);
};