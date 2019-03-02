#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"

#define MAX_NUM_ACTOR_SHAPES_OBJ 128

PhysXObject::PhysXObject(PxRigidActor* ra)
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

	const PxU32 nb_shapes = m_ra->getNbShapes();
	PX_ASSERT(nb_shapes <= MAX_NUM_ACTOR_SHAPES_OBJ);
	m_ra->getShapes(shapes, nb_shapes);
	bool is_sleeping = m_ra->is<PxRigidDynamic>() ? m_ra->is<PxRigidDynamic>()->isSleeping() : false; 

	for (PxU32 i = 0; i < nb_shapes; ++i)
	{
		const PxMat44 shape_pose(PxShapeExt::getGlobalPose(*shapes[i], *m_ra));
		PxGeometryHolder h = shapes[i]->getGeometry();

		std::string shader_name = "default";
		const std::string tex_name = "Data/Images/white.png";
		const std::string mesh_name = ChooseMesh(h);

		bool is_trigger = false;
		if (shapes[i]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
			is_trigger = true;

		// get potentially different shader based on sleep and trigger state
		ChooseShader(is_sleeping, is_trigger, shader_name);

		rdr->RenderPhysxGeometry(mesh_name, shader_name, tex_name, shape_pose);
	}
}

std::string PhysXObject::ChooseMesh(const PxGeometryHolder& h)
{
	return "";
}

void PhysXObject::ChooseShader(const bool& is_sleep, const bool& is_trigger, std::string& sn)
{

}
