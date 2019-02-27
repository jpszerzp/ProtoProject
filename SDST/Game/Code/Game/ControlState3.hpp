#pragma once

#include "Game/GameState.hpp"
#include "Engine/Physics/3D/RF/CollisionPrimitive.hpp"
#include "Engine/Physics/3D/RF/CollisionKeep.hpp"

class ControlState3 : public GameState
{
public:
	ControlState3();
	~ControlState3();

	void Update(float deltaTime) override;
	void UpdateMouse(float deltaTime) override;
	void UpdateKeyboard(float deltaTime) override; 
	void UpdateDebugDraw(float deltaTime);
	void UpdateObjects(float deltaTime);
	void UpdateContacts(float deltaTime);
	void UpdateUI(float deltaTime);

	void Render(Renderer* renderer) override;

public:
	const static uint MAX_CONTACT_NUM = 256;

private:
	std::vector<CollisionPlane*> m_planes;
	std::vector<CollisionConvexObject*> m_convex_objs;
	CollisionPrimitive* m_controlled_0;
	CollisionKeep m_keep;
	Collision m_storage[MAX_CONTACT_NUM];

	Mesh* m_title_ui;
};
