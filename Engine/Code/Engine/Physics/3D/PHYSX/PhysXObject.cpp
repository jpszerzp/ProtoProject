/*
#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"
#include "Engine/Core/Util/AssetUtils.hpp"

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

Vector3 PhysXObject::GetLinearVel() const
{
	PxRigidDynamic* dyn = static_cast<PxRigidDynamic*>(m_ra);
	PxVec3 vel = dyn->getLinearVelocity();
	return Vector3(vel.x, vel.y, vel.z);
}

Vector3 PhysXObject::GetAngularVel() const
{
	PxRigidDynamic* dyn = static_cast<PxRigidDynamic*>(m_ra);
	PxVec3 vel = dyn->getAngularVelocity();
	return Vector3(vel.x, vel.y, vel.z);
}

Vector3 PhysXObject::GetPos() const
{
	PxVec3 pos = m_ra->getGlobalPose().p;
	return Vector3(pos.x, pos.y, pos.z);
}

void PhysXObject::SetPos(const Vector3& pos)
{
	PxVec3 p = PxVec3(pos.x, pos.y, pos.z);
	PxTransform new_t = PxTransform(p, m_ra->getGlobalPose().q);
	m_ra->setGlobalPose(new_t);
}

void PhysXObject::SetOrient(const Quaternion& orient)
{
	// make sure it is normalize
	PxQuat q = PxQuat(orient.m_imaginary.x, orient.m_imaginary.y, orient.m_imaginary.z, orient.m_real);
	q.normalize();
	PxTransform new_t = PxTransform(m_ra->getGlobalPose().p, q);
	m_ra->setGlobalPose(new_t);
}

void PhysXObject::SetPosAndOrient(const Vector3& pos, const Quaternion& orient)
{
	// pos
	PxVec3 p = PxVec3(pos.x, pos.y, pos.z);
	// quat
	PxQuat q = PxQuat(orient.m_imaginary.x, orient.m_imaginary.y, orient.m_imaginary.z, orient.m_real);
	q.normalize();
	PxTransform new_t = PxTransform(p, q);
	m_ra->setGlobalPose(new_t);
}

void PhysXObject::SetLinearVel(const Vector3& lin_vel)
{
	// assuming the object set velocity with is dynamic
	PxRigidDynamic* dyn = static_cast<PxRigidDynamic*>(m_ra);
	PxVec3 vel = PxVec3(lin_vel.x, lin_vel.y, lin_vel.z);
	dyn->setLinearVelocity(vel);
}

void PhysXObject::SetAngularVel(const Vector3& ang_vel)
{
	// assuming the object set veloicty with is dynamic
	PxRigidDynamic* dyn = static_cast<PxRigidDynamic*>(m_ra);
	PxVec3 vel = PxVec3(ang_vel.x, ang_vel.y, ang_vel.z);
	dyn->setAngularVelocity(vel);
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

		bool is_trigger = false;
		if (shapes[i]->getFlags() & PxShapeFlag::eTRIGGER_SHAPE)
			is_trigger = true;

		// default
		std::string shader_name = "default";
		std::string tex_name = GetAbsImgPath("white");

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

void PhysXObject::RenderActor(Renderer* rdr, const std::string& shader, const std::string& tex)
{
	PxShape* shapes[MAX_NUM_ACTOR_SHAPES_OBJ];

	const PxU32 nb_shapes = m_ra->getNbShapes();
	PX_ASSERT(nb_shapes <= MAX_NUM_ACTOR_SHAPES_OBJ);
	m_ra->getShapes(shapes, nb_shapes);

	for (PxU32 i = 0; i < nb_shapes; ++i)
	{
		const PxMat44 shape_pose(PxShapeExt::getGlobalPose(*shapes[i], *m_ra));
		PxGeometryHolder h = shapes[i]->getGeometry();

		// render config
		const std::string& mesh_name = ChooseMesh(h);

		// at this point, for static bodies, do not render, treat it as a block of fixed "air"
		bool is_dynamic = m_ra->is<PxRigidDynamic>();
		if (is_dynamic)
			rdr->RenderPhysxGeometry(mesh_name, shader, tex, shape_pose);
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

	return "";
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
		tn = GetAbsImgPath("white");
	// non sleep but trigger
	else if (is_trigger)
		tn = GetAbsImgPath("white");
	// non sleep non trigger
	else
		tn = GetAbsImgPath("orient");
}

void PhysXObject::DisableGravity()
{
	m_ra->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
}
*/