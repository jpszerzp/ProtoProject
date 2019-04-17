#pragma once

#include "Game/GameState.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"
#include "Engine/Core/HullObject.hpp"

enum eCID
{
	CID_PLANE,
	CID_BOX,
	CID_SPHERE,
	CID_CONVEX,
	CID_LINE,
	CID_AABB,
};

class ControlState3 : public GameState
{
public:
	ControlState3();
	~ControlState3();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override; 
	void UpdateDebugDraw(float deltaTime);
	void UpdatePair(float deltaTime);
	void UpdateContacts(float deltaTime);
	void UpdateUI(float deltaTime);

	void Render(Renderer* renderer) override;
	void RenderPair(Renderer* renderer);

public:
	const static uint MAX_CONTACT_NUM = 256;

private:
	eCID m_cid_0 = CID_PLANE;
	eCID m_cid_1 = CID_PLANE;

	// control 0
	CollisionBox* m_box_0;
	CollisionBox* m_aabb_0;
	CollisionSphere* m_sphere_0;
	CollisionPlane* m_plane_0;
	CollisionLine* m_line_0;
	CollisionConvexObject* m_convex_0;
	CollisionPrimitive* m_controlled_0;

	// control 1
	CollisionBox* m_box_1;
	CollisionBox* m_aabb_1;
	CollisionSphere* m_sphere_1;
	CollisionPlane* m_plane_1;
	CollisionLine* m_line_1;
	CollisionConvexObject* m_convex_1;
	CollisionPrimitive* m_controlled_1;

	CollisionKeep m_keep;
	Collision m_storage[MAX_CONTACT_NUM];

	QuickHull* m_qh = nullptr;

	HullObject* m_hobj_0;
	HullObject* m_hobj_1;

	std::vector<Vector3> m_inspection;
	int m_insepction_count = 0;

	// origin
	Mesh* m_origin;

	// UI
	Vector2 m_start_min;
	float m_text_height;
	Mesh* m_title_ui;
	std::vector<Mesh*> m_dynamic_ui;
};