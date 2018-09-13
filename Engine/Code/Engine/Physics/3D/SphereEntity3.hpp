#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/Sphere3.hpp"

class SphereEntity3 : public Entity3
{
private:
	Sphere3 m_primitive;

public:
	SphereEntity3(const Sphere3& primitive, bool isConst = false);
	~SphereEntity3();

	void Update(float deltaTime) override;
	void Render(Renderer* renderer) override;
	void Translate(Vector3 translation) override;

	Sphere3& GetSpherePrimitive() { return m_primitive; }

	void SetEntityForPrimitive() override;
};