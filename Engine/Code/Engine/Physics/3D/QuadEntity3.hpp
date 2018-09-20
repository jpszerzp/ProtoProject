#pragma once

#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Math/Plane.hpp"


class QuadEntity3 : public Entity3
{
private:
	Plane m_primitive;

	// because plane primitive is special: it does mot bare with pos/rot/scale info
	//Vector3 m_quadCenter;
	//Vector3 m_quadRot;
	//Vector3 m_quadScale;

public:
	//QuadEntity3(const Plane& primitive, bool isConst, Vector3 center, Vector3 rot, Vector3 scale);
	QuadEntity3(const Plane& primitive, eMoveStatus moveStat,
		Vector3 center, Vector3 rot, Vector3 scale);
	~QuadEntity3();

	void UpdateEntityPrimitive() override;
	void Render(Renderer* renderer) override;
	void Translate(Vector3 translation) override;

	Plane& GetPlanePrimitive() { return m_primitive; }

	void SetEntityForPrimitive() override;
};