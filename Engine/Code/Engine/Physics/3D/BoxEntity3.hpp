#pragma once
#include "Engine/Physics/3D/Entity3.hpp"
#include "Engine/Physics/3D/CollisionDetection.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Line3.hpp"

class BoxEntity3 : public Entity3
{
private:
	OBB3 m_primitive;

public:
	eContactFeature m_features[26] = {
		V1, V2, V3, V4, V5, V6, V7, V8,
		F1, F2, F3, F4, F5, F6,
		E1, E2, E3, E4, E5, E6, E7, E8, E9, E10, E11, E12
	};

public:
	BoxEntity3(const OBB3& primitive, const Vector3& rot, eMoveStatus moveStat);
	~BoxEntity3();

	Vector3 GetFeaturedPoint(eContactFeature feature);
	LineSegment3 GetFeaturedEdge(eContactFeature feature);

	void UpdateEntityPrimitive() override;
	void Render(Renderer* renderer) override;

	const OBB3& GetBoxPrimitive() const { return m_primitive; }

	void SetEntityForPrimitive() override;
};