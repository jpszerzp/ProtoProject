/*
#include "Engine/Physics/3D/PHYSX/PhysXObject.hpp"

#define MAX_NUM_ACTOR_SHAPES_OBJ 128

PhysXObject::PhysXObject(PxRigidActor* ra)
	: m_ra(ra)
{
	PX_ASSERT(m_ra != nullptr);

	// get shape data
	PxShape* shapes[MAX_NUM_ACTOR_SHAPES_OBJ];
	CONST PxU32 nb_shapes = m_ra->getNbShapes();
	PX_ASSERT(nb_shapes <= MAX_NUM_ACTOR_SHAPES_OBJ);
	m_ra->getShapes(shapes, nb_shapes);

	// store the geometry
	for (int i = 0; i < nb_shapes; ++i)
	{
		PxShape* shp = shapes[i];

		// get shape's model matrix
		const PxMat44 shape_pose(PxShapeExt::getGlobalPose(*shp, *m_ra));
		
		// get shape's geometry
		const PxGeometryHolder& h = shp->getGeometry();

		// create shape render class, store it
		PhysXShapeRender shp_rdr = PhysXShapeRender(h, shape_pose);
		m_shape_renders.push_back(shp_rdr);
	}
	// each shape render will handle its own mesh, shader and texture
}

PhysXObject::~PhysXObject()
{
	// destroy of ra will be handled by physx system from within...
	// ...
}

void PhysXObject::RenderActor(Renderer* rdr)
{

}

void PhysXObject::Render(Renderer* rdr)
{
	for (int i = 0; i < m_shape_renders.size(); ++i)
		m_shape_renders[i].Render(rdr);
}

void PhysXObject::CacheData()
{
	//for (int i = 0; i < m_shape_renders.size(); ++i)

}
*/