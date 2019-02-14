#include "Engine/Physics/3D/RF/ConvexHull.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

ConvexHull::ConvexHull(const std::vector<Plane>& planes)
	: m_planes(planes)
{

}

Plane ConvexHull::GetPlane(uint idx) const
{
	ASSERT_OR_DIE(idx < m_planes.size(), "Plane idx out of range");

	return m_planes[idx];
}

uint ConvexHull::GetPlaneNum() const
{
	return (uint)m_planes.size();
}

//void ConvexHull::ImportMassSampleData()
//{
//	// mass sample locations
//	// ...
//
//	AddSample( Vector3(0.f, 0.f, 0.f)      );
//	AddSample( Vector3(5.f, 0.f, 0.f)      );
//	AddSample( Vector3(-5.f, 0.f, 0.f)     );
//	AddSample( Vector3(0.f, 0.f, -3.f)     );
//	AddSample( Vector3(0.f, 0.f, -6.f)     );
//	AddSample( Vector3(0.f, 0.f, 2.f)      );
//	AddSample( Vector3(0.f, 0.f, 4.f)      );
//	AddSample( Vector3(0.f, -3.f, 0.f)     );
//	AddSample( Vector3(0.f, -4.5f, 0.f)    );
//	AddSample( Vector3(0.f, 2.f, 0.f)      );
//	AddSample( Vector3(0.f, 4.f, 0.f)      );
//	AddSample( Vector3(7.f, 0.f, 0.f)      );
//	AddSample( Vector3(-7.f, 0.f, 0.f)     );
//	AddSample( Vector3(3.f, 1.5f, 2.f)     );
//	AddSample( Vector3(3.f, -1.5f, 2.f)    );
//	AddSample( Vector3(-3.f, -1.5f, 2.f)   );
//	AddSample( Vector3(-3.f, 1.5f, 2.f)    );
//	AddSample( Vector3(3.f, 2.f, 3.f)      );
//	AddSample( Vector3(4.f, -2.f, 3.f)     );
//	AddSample( Vector3(-4.f, -2.f, 3.f)    );
//	AddSample( Vector3(-3.f, 2.f, 3.f)     );
//	AddSample( Vector3(5.f, -2.f, -5.f)    );
//	AddSample( Vector3(5.f, 2.f, -4.f)     );
//	AddSample( Vector3(-5.f, 2.f, -4.f)    );
//	AddSample( Vector3(-5.f, -2.f, -5.f)   );
//	AddSample( Vector3(8.f, -3.3f, -7.f)   );
//	AddSample( Vector3(-8.f, -3.3f, -7.f)  );
//	AddSample( Vector3(13.f, -4.1f, -9.f)  );
//	AddSample( Vector3(-13.f, -4.1f, -9.f) );
//	AddSample( Vector3(0.f, -2.5f, 7.5f)   );
//}
