#pragma once

#include "Engine/Physics/2D/Entity.hpp"
#include "Engine/Math/OBB2.hpp"

class RectangleEntity : public Entity
{
public:
	Transform m_rectangleTransform;
	OBB2	  m_primitive;
	//bool	  m_dead = false;
	//Mesh*	  m_boundRecMesh = nullptr;

public:
	RectangleEntity(const RectangleEntity& copy);
	RectangleEntity(const OBB2& primitive, bool isConst = false);
	~RectangleEntity();

	void Update(float deltaTime) override;
	void UpdateInput() override;
	void UpdateBoundAABB() override;
	void UpdateBoundDisc() override;
	void UpdateTransformFinal();

	void WrapAround();

	void Render(Renderer* renderer) override;

	OBB2 GetQuadPrimitive() const { return m_primitive; }
};