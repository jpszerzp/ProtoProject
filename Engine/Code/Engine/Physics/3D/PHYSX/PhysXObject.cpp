#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"

#define MAX_NUM_ACTOR_SHAPES_OBJ 128

PhysXObject::PhysXObject(PxRigidActor* ra)
//PhysXObject::PhysXObject(PxActor* ra)
	: m_ra(ra)
{

}

PhysXObject::~PhysXObject()
{
	// destroy of ra will be handled by physx system from within...
	// ...
}

void PhysXObject::RenderActor(Renderer* rdr)
{
	PxShape* shapes[MAX_NUM_ACTOR_SHAPES_OBJ];

	//PxRigidActor* converted_ra = dynamic_cast<PxRigidActor*>(m_ra);
	const PxU32 nb_shapes = m_ra->getNbShapes();
	//const PxU32 nb_shapes = converted_ra->getNbShapes();
	PX_ASSERT(nb_shapes <= MAX_NUM_ACTOR_SHAPES_OBJ);
	m_ra->getShapes(shapes, nb_shapes);
	//converted_ra->getShapes(shapes, nb_shapes);
	bool is_sleeping = m_ra->is<PxRigidDynamic>() ? m_ra->is<PxRigidDynamic>()->isSleeping() : false; 

	for (PxU32 i = 0; i < nb_shapes; ++i)
	{
		const PxMat44 shape_pose(PxShapeExt::getGlobalPose(*shapes[i], *m_ra));
		//const PxMat44 shape_pose(PxShapeExt::getGlobalPose(*shapes[i], *converted_ra));
		PxGeometryHolder h = shapes[i]->getGeometry();

		bool is_trigger = false;
		if (shapes[i]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
			is_trigger = true;

		// default
		std::string shader_name = "default";
		std::string tex_name = "Data/Images/white.png";

		// render config
		const std::string& mesh_name = ChooseMesh(h);
		ChooseShader(is_sleeping, is_trigger, shader_name);
		ChooseTexture(is_sleeping, is_trigger, tex_name);

		// at this point, for static bodies, do not render, treat it as a block of fixed "air"
		bool is_dynamic = m_ra->is<PxRigidDynamic>();

		if (is_dynamic)
			rdr->RenderPhysxGeometry(mesh_name, shader_name, tex_name, shape_pose);
	}
}

std::string PhysXObject::ChooseMesh(const PxGeometryHolder& h)
{
	switch (h.getType())
	{
	case PxGeometryType::eBOX:			
	{
		return "cube_pcu";
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
		break;
	case PxGeometryType::ePLANE:
	{
		return "quad_pcu_20";
	}
		break;
	default:
		break;
	}
}

void PhysXObject::ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn)
{
	// sleep
	if (is_sleep)
		sn = "plain_white";

	// non sleep but trigger
	else if (is_trigger)
		sn = "wireframe";

	// non sleep non trigger
	else
		sn = "default";
}

void PhysXObject::ChooseTexture(const bool& is_sleep, const bool& is_trigger, std::string& tn)
{
	// sleep
	if (is_sleep)
		tn = "Data/Images/white.png";

	// non sleep but trigger
	else if (is_trigger)
		tn = "Data/Images/white.png";

	// non sleep non trigger
	else
		tn = "Data/Images/perspective_test.png";
}
