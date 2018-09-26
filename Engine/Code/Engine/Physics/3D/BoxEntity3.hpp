#pragma once
#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/OBB3.hpp"

class BoxEntity3 : public Entity3
{
private:
	OBB3 m_primitive;

public:
	BoxEntity3(const OBB3& primitive, const Vector3& rot, eMoveStatus moveStat);
	~BoxEntity3();

	void UpdateEntityPrimitive() override;
	void Render(Renderer* renderer) override;

	const OBB3& GetBoxPrimitive() const { return m_primitive; }

	void SetEntityForPrimitive() override;
};