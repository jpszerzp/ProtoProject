#pragma once

#include "Game/GameState.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

enum eCID
{
	CID_PLANE,
	CID_BOX,
	CID_SPHERE,
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
	eCID m_cid = CID_PLANE;

	std::vector<CollisionPlane*> m_planes;
	std::vector<CollisionSphere*> m_spheres;
	std::vector<CollisionBox*> m_boxes;
	std::vector<CollisionConvexObject*> m_convex_objs;

	CollisionPrimitive* m_controlled_0;
	CollisionPrimitive* m_controlled_1;

	CollisionKeep m_keep;
	Collision m_storage[MAX_CONTACT_NUM];

	Mesh* m_title_ui;

	QuickHull* m_qh = nullptr;
};