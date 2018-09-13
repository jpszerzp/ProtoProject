#pragma once

#include "Engine/Physics/2D/Entity.hpp"

class DiscEntity : public Entity
{
public:
	Transform	m_discTransform;
	Disc2		m_primitive;
	Mesh*		m_orientMesh = nullptr;
	bool		m_kinematic = false;

public:
	DiscEntity(const DiscEntity& copy);
	DiscEntity(const Disc2& primitive, bool isConst = false);
	~DiscEntity();

	void Update(float deltaTime) override;
	void UpdateInput() override;
	void UpdateBoundAABB() override;
	void UpdateBoundDisc() override;
	void UpdateTransformFinal();

	void WrapAround();

	void Render(Renderer* renderer) override;

	bool CollideWithDiscEntity(DiscEntity& discEntity);
	bool CollideWithDiscEntity(DiscEntity& discEntity, Vector2& overlap);
	//bool ColildeWithCapsuleEntity(CapsuleEntity& capsuleEntity, Vector2& overlap, eCapsuleRegion& region);

	Disc2		GenerateDisc2();
	float		GetDisc2Radius();
	Vector2     GetDisc2Center();
};