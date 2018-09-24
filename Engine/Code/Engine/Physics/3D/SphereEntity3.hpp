#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/Sphere3.hpp"

class SphereEntity3 : public Entity3
{
private:
	Sphere3 m_primitive;

public:
	SphereEntity3(const Sphere3& primitive, eMoveStatus moveStat);
	~SphereEntity3();

	void UpdateEntityPrimitive() override;
	void Render(Renderer* renderer) override;
	void Translate(Vector3 translation) override;

	const Sphere3& GetSpherePrimitive() const { return m_primitive; }

	void SetEntityForPrimitive() override;
};