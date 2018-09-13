#pragma once


#include "Engine/Physics/2D/Entity.hpp"

class QuadEntity : public Entity
{
public:
	Transform m_quadTransform;
	AABB2	  m_primitive;

	bool m_kinematic = false;
	bool m_continued = false;

public:
	QuadEntity(const QuadEntity& copy);
	QuadEntity(const AABB2& primitive, bool isConst = false);
	~QuadEntity();

	void Update(float deltaTime) override;
	void UpdateInput() override;
	void UpdateBoundDisc() override;
	void UpdateBoundAABB() override;
	void UpdateTransformFinal();

	void Integrate(float deltaTime) override;

	void WrapAround();

	void Render(Renderer* renderer) override;

	AABB2 GetQuadPrimitive() const { return m_primitive; }
};