#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/AABB3.hpp"

class CubeEntity3 : public Entity3
{
private:
	AABB3 m_primitive;

public:
	CubeEntity3(const AABB3& primitive, eMoveStatus moveStat);
	~CubeEntity3();

	void UpdateEntityPrimitive() override;
	void Render(Renderer* renderer) override;

	void Translate(Vector3 translation) override;

	const AABB3& GetCubePrimitive() const { return m_primitive; }

	void SetEntityForPrimitive() override;
};