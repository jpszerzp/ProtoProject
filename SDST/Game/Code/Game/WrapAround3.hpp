#pragma once

#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/GameObject.hpp"
#include "Engine/Physics/3D/Rigidbody3.hpp"
#include "Engine/Physics/3D/BVH3.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"

#include <vector>

#define BVH_CONTACT_LIMIT 1000

class Physics3State;

class WrapAround
{
public:
	AABB3 m_bounds;
	std::vector<CollisionPrimitive*> m_primitives;
	Mesh* m_mesh = nullptr;
	Transform m_transform;
	Vector3 m_positions[8];
	int m_pos_idx = 0;

	//bvh
	int m_bvh_node_count = 0;
	bool m_bvh_based = false;
	BVHNode<BoundingSphere>* m_bvh = nullptr;
	std::vector<BVHContact> m_bvh_contacts;

	Physics3State* m_physState = nullptr;
	
public:
	WrapAround(const Vector3& min, const Vector3& max, 
		const Vector3& p1 = Vector3::ZERO, const Vector3& p2 = Vector3::ZERO, 
		const Vector3& p3 = Vector3::ZERO, const Vector3& p4 = Vector3::ZERO,
		const Vector3& p5 = Vector3::ZERO, const Vector3& p6 = Vector3::ZERO,
		const Vector3& p7 = Vector3::ZERO, const Vector3& p8 = Vector3::ZERO);
	~WrapAround();

	void Update();
	void UpdateInput();
	void UpdateBVH();
	void UpdateBVHContactGeneration();			// used in phyics state
	void UpdateWraparound();

	void Render(Renderer* renderer);
	void RenderBVH(Renderer* renderer);
	void RenderWraparounds(Renderer* renderer);

	void ClearBVHRecords();

	void RemovePrimitive(CollisionPrimitive* prim);
};