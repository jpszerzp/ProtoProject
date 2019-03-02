/*
#include "Engine/Physics/3D/PHYSX/PhysXShapeRender.hpp"

PhysXShapeRender::PhysXShapeRender(const PxGeometryHolder& h, const PxMat44& mm)
	: m_geo(h), m_model_matrix(mm)
{
	// based on geometry (or other conditions), initialize mesh, shader and texture as you wish
	// ...

}

PhysXShapeRender::~PhysXShapeRender()
{

}

void PhysXShapeRender::Render(Renderer* rdr)
{
	switch (m_geo.getType())
	{
	case PxGeometryType::eBOX:
	{
		// a box...

	}
	break;
	case PxGeometryType::eSPHERE:		
	{

	}
	break;
	case PxGeometryType::eCAPSULE:
	{

	}
	break;
	case PxGeometryType::eCONVEXMESH:
	{

	}
	break;
	case PxGeometryType::eTRIANGLEMESH:
	{

	}
	break;
	case PxGeometryType::eINVALID:
	case PxGeometryType::eHEIGHTFIELD:
	case PxGeometryType::eGEOMETRY_COUNT:	
	case PxGeometryType::ePLANE:
	{
		// a plane...

	}
	break;
	default:
		break;
	}
}
*/