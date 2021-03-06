//#include "Engine/Physics/3D/CollisionDetection.hpp"
//#include "Engine/Physics/3D/CubeEntity3.hpp"
//#include "Engine/Physics/3D/QuadEntity3.hpp"
//#include "Engine/Physics/3D/PointEntity3.hpp"
//#include "Engine/Physics/3D/BoxEntity3.hpp"
//#include "Engine/Physics/3D/BoxRB3.hpp"
//#include "Engine/Physics/3D/SphereRB3.hpp"
//#include "Engine/Physics/3D/QuadRB3.hpp"
//#include "Engine/Core/GameObject.hpp"
//#include "Engine/Core/Util/DataUtils.hpp"
//#include "Engine/Math/MathUtils.hpp"
//#include "Engine/Math/Line3.hpp"
//#include "Engine/Renderer/DebugRenderer.hpp"
//#include "Engine/Input/InputSystem.hpp"
//
//#define INVALID_DEPTH_BOX_TO_POINT -1.f
//#define INVALID_DEPTH_EDGE_TO_EDGE -1.f
//
//enum eBoxBoxDebug
//{
//	SECOND_OBB_VERTS,
//	FIRST_OBB_VERTS,
//	TWO_OBB_EDGES
//};
//
//Contact3::Contact3()
//{
//
//}
//
//
//Contact3::Contact3(Entity3* e1, Entity3* e2)
//{
//	m_e1 = e1;
//	m_e2 = e2;
//}
//
//Contact3::Contact3(Entity3* e1, Entity3* e2, Vector3 normal, Vector3 point, float penetration, float res, float friction)
//{
//	m_e1 = e1;
//	m_e2 = e2;
//	m_normal = normal;
//	m_point = point;
//	m_penetration = penetration;
//	m_restitution = res;
//	m_friction = friction;
//}
//
//Contact3::~Contact3()
//{
//
//}
//
//
//void Contact3::ResolveContact(float deltaTime)
//{
//	ResolveVelocity(deltaTime);
//	ResolvePenetration(deltaTime);
//}
//
//void Contact3::MakeToWorld()
//{
//	Vector3 tangent[2];		// tangent[0] is y and the other z
//
//	if (abs(m_normal.x) > abs(m_normal.y))
//	{
//		float s = 1.f / sqrtf(m_normal.z * m_normal.z + m_normal.x * m_normal.x);
//
//		tangent[0].x = m_normal.z * s;
//		tangent[0].y = 0.f;
//		tangent[0].z = -m_normal.x * s;
//
//		tangent[1].x = m_normal.y * tangent[0].x;
//		tangent[1].y = m_normal.z * tangent[0].x - m_normal.x * tangent[0].z;
//		tangent[1].z = -m_normal.y * tangent[0].x;
//	}
//	else 
//	{
//		float s = 1.f / sqrtf(m_normal.z * m_normal.z + m_normal.y * m_normal.y);
//
//		tangent[0].x = 0.f;
//		tangent[0].y = -m_normal.z * s;
//		tangent[0].z = m_normal.y * s;
//
//		tangent[1].x = m_normal.y * tangent[0].z - m_normal.z * tangent[0].y;
//		tangent[1].y = -m_normal.x * tangent[0].z;
//		tangent[1].z = m_normal.x * tangent[0].y;
//	}
//
//	m_toWorld.SetBasis(m_normal, tangent[0], tangent[1]);
//}
//
//float Contact3::GetVelPerImpulseContact()
//{
//	Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//
//	Vector3 velWorld = m_relativePosWorld[0].Cross(m_normal);
//	velWorld = rigid1->m_inverseInertiaTensorWorld * velWorld;
//	velWorld = velWorld.Cross(m_relativePosWorld[0]);
//
//	// orthonormal basis means we can use transpose for inverse
//	// get velocity in contact coord - the shortcut, see p317 of GPED
//	float velContact = 0.f;
//	if (!rigid1->IsEntityStatic() && !rigid1->IsEntityKinematic())
//	{
//		velContact = DotProduct(velWorld, m_normal);		// angular
//		velContact += rigid1->GetMassData3().m_invMass;		// linear
//	}
//
//	// second body
//	if (m_e2 != nullptr)
//	{
//		Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e2);
//
//		velWorld = m_relativePosWorld[1].Cross(m_normal);
//		velWorld = rigid2->m_inverseInertiaTensorWorld * velWorld;
//		velWorld = velWorld.Cross(m_relativePosWorld[1]);
//
//		if (!rigid2->IsEntityStatic() && !rigid2->IsEntityKinematic())
//		{
//			velContact += DotProduct(velWorld, m_normal);
//			velContact += rigid2->GetMassData3().m_invMass;
//		}
//	}
//
//	return velContact;
//}
//
//float Contact3::GetDeltaVel()
//{
//	Vector3 relPos1 = m_point - m_e1->GetEntityCenter();
//	Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//	Vector3 vel = Vector3::ZERO;
//	if (rigid1 != nullptr)
//	{
//		// contrib from 1
//		vel += rigid1->GetAngularVelocity().Cross(relPos1);	// angular
//		vel += rigid1->GetLinearVelocity();					// linear
//	}
//
//	Vector3 relPos2 = m_point - m_e2->GetEntityCenter();
//	Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e2);
//	if (rigid2 != nullptr)
//	{
//		vel += rigid2->GetAngularVelocity().Cross(relPos2);
//		vel += rigid2->GetLinearVelocity();
//	}
//
//	ASSERT_OR_DIE(false, "GetDeltaVel is deprecated, ignore this function");
//	Matrix33 contactToWorld;
//	//MakeToWorld(contactToWorld);
//	Matrix33 worldToContact = contactToWorld.GetTranspose();
//	Vector3 contactVel = worldToContact * vel;
//
//	float deltaVel = -(1.f + m_restitution) * contactVel.x;
//	// try 0.4 for restitution
//
//	return deltaVel;
//}
//
//Vector3 Contact3::ComputeContactImpulse()
//{
//	float delta = GetVelPerImpulseContact();
//	float imp = m_desiredVelDelta / delta;
//	return Vector3(imp, 0.f, 0.f);
//}
//
//Vector3 Contact3::ComputeContactImpulseFriction()
//{
//	Vector3 imp;
//	Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//
//	float invMass = 0.f;
//	Matrix33 deltaVelWorld1;
//	Matrix33 deltaVelWorld2;
//	Matrix33 deltaVelWorld = Matrix33(0.f);
//	Matrix33 toTorque;
//
//	if (!rigid1->IsEntityStatic() && !rigid1->IsEntityKinematic())
//	{
//		invMass += rigid1->GetMassData3().m_invMass;
//
//		toTorque.SetSkewSymmetric(m_relativePosWorld[0]);
//
//		deltaVelWorld1 = toTorque;
//		deltaVelWorld1 *= rigid1->m_inverseInertiaTensorWorld;
//		deltaVelWorld1 *= toTorque;
//		deltaVelWorld1 *= -1.f;
//
//		deltaVelWorld += deltaVelWorld1;
//	}
//
//	if (m_e2 != nullptr)
//	{
//		Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e1);
//
//		if (!rigid2->IsEntityStatic() && !rigid2->IsEntityKinematic())
//		{
//			// it was a cross product in the frictionless version
//			toTorque.SetSkewSymmetric(m_relativePosWorld[1]);
//
//			deltaVelWorld2 = toTorque;
//			deltaVelWorld2 *= rigid2->m_inverseInertiaTensorWorld;
//			deltaVelWorld2 *= toTorque;
//			deltaVelWorld2 *= -1.f;
//
//			deltaVelWorld += deltaVelWorld2;
//
//			invMass += rigid2->GetMassData3().m_invMass;
//		}
//	}
//
//	// convert to contact coord (change of basis)
//	Matrix33 deltaVelocity = m_toWorld.GetTranspose();
//	deltaVelocity *= deltaVelWorld;
//	deltaVelocity *= m_toWorld;
//	
//	// linear velocity change as a matrix
//	deltaVelocity.Ix += invMass;
//	deltaVelocity.Jy += invMass;
//	deltaVelocity.Kz += invMass;
//
//	// impulse per velocity
//	Matrix33 impulseMatrix = deltaVelocity.GetInverse();
//
//	// velocity to kill by friction
//	Vector3 toKill(m_desiredVelDelta, -m_closingVel.y, -m_closingVel.z);
//
//	// impulse needed for the kill
//	imp = impulseMatrix * toKill;
//
//	// should we use dynamic friction?
//	float planarImp = sqrtf(imp.y * imp.y + imp.z * imp.z);
//	if (planarImp > imp.x * m_friction)
//	{
//		imp.y /= planarImp;
//		imp.z /= planarImp;
//
//		imp.x = deltaVelocity.Ix + deltaVelocity.Jx * m_friction * imp.y + deltaVelocity.Kx * m_friction * imp.z;
//		imp.x = m_desiredVelDelta / imp.x;
//		imp.y *= m_friction * imp.x;
//		imp.z *= m_friction * imp.x;
//	}
//
//	return imp;
//}
//
//Vector3 Contact3::ComputeWorldImpulse()
//{
//	return m_toWorld * ComputeContactImpulse();
//}
//
//Vector3 Contact3::ComputeWorldImpulseFriction()
//{
//	Vector3 worldImpulse = m_toWorld * ComputeContactImpulseFriction();
//	DebugRenderLine(0.2f, m_point, m_point + worldImpulse, 5.f, Rgba::CYAN, Rgba::CYAN, DEBUG_RENDER_USE_DEPTH);
//	return worldImpulse;
//}
//
//// the goal is to transform from world space to contact space
//Vector3 Contact3::RF_ComputeFrictionlessImpulse()
//{
//	Vector3 impulseContact;
//
//	// assumes the normal is normalized
//	Vector3 deltaVelWorld = m_relativePosWorld[0].Cross(m_normal);
//	deltaVelWorld = m_e1->GetIITWorld() * deltaVelWorld;
//	deltaVelWorld = deltaVelWorld.Cross(m_relativePosWorld[0]);
//
//	float deltaVelocity = DotProduct(deltaVelWorld, m_normal);
//
//	deltaVelocity += m_e1->m_massData.m_invMass;
//
//	if (m_e2 != nullptr)
//	{
//		deltaVelWorld = m_relativePosWorld[1].Cross(m_normal);
//		deltaVelWorld = m_e2->GetIITWorld() * deltaVelWorld;
//		deltaVelWorld = deltaVelWorld.Cross(m_relativePosWorld[1]);
//
//		// to contact coord (for angular)
//		deltaVelocity += DotProduct(deltaVelWorld, m_normal);
//
//		// for linear
//		deltaVelocity += m_e2->m_massData.m_invMass;
//	}
//
//	// in frictionless case, we only care about the normal direction, which is x of impulse
//	impulseContact.x = m_desiredVelDelta / deltaVelocity;
//	impulseContact.y = 0.f;
//	impulseContact.z = 0.f;
//	return impulseContact;
//}
//
//Vector3 Contact3::RF_ComputeFrictionalImpulse()
//{
//	Vector3 impulse_contact;
//	float im = m_e1->GetInverseMass();
//
//	// use skewmatrix to commit cross product
//	Matrix33 impulse_to_torque;
//	impulse_to_torque.SetSkewSymmetric(m_relativePosWorld[0]);
//
//	Matrix33 del_vel_world = impulse_to_torque;
//	del_vel_world *= m_e1->GetInverseInertiaTensor();
//	del_vel_world *= impulse_to_torque;
//	del_vel_world *= -1;
//
//	if (m_e2 != nullptr)
//	{
//		impulse_to_torque.SetSkewSymmetric(m_relativePosWorld[1]);
//
//		Matrix33 del_vel_world_other = impulse_to_torque;
//		del_vel_world_other *= m_e2->GetInverseInertiaTensor();
//		del_vel_world_other *= impulse_to_torque;
//		del_vel_world_other *= -1;
//
//		del_vel_world += del_vel_world_other;
//
//		im += m_e2->GetInverseMass();
//	}
//
//	Matrix33 del_vel = m_toWorld.GetTranspose();
//	del_vel *= del_vel_world;
//	del_vel *= m_toWorld;
//
//	del_vel.Ix += im;
//	del_vel.Jy += im;
//	del_vel.Kz += im;
//
//	Matrix33 impulse_mat = del_vel.GetInverse();
//
//	Vector3 vel_cancel = Vector3(m_desiredVelDelta, -m_closingVel.y, -m_closingVel.z);
//
//	impulse_contact = impulse_mat * vel_cancel;
//
//	// static and dynamic friction
//	float planar_impulse =  sqrtf(impulse_contact.y * impulse_contact.y + impulse_contact.z * impulse_contact.z);
//	if (planar_impulse > impulse_contact.x * m_friction)
//	{
//		impulse_contact.y /= planar_impulse;
//		impulse_contact.z /= planar_impulse;
//
//		impulse_contact.x = del_vel.Ix + del_vel.Jx * m_friction * impulse_contact.y + del_vel.Kx * m_friction * impulse_contact.z;
//		impulse_contact.x = m_desiredVelDelta / impulse_contact.x;
//		impulse_contact.y *= m_friction * impulse_contact.x;
//		impulse_contact.z *= m_friction * impulse_contact.x;
//	}
//
//	return impulse_contact;
//}
//
//void Contact3::ApplyImpulse()
//{
//	Vector3 pos_linearChange[2]; 
//	Vector3 pos_angularChange[2];
//	Vector3 vel_linearChange[2]; 
//	Vector3 vel_angularChange[2];
//	ResolveVelocityCoherent(vel_linearChange, vel_angularChange);
//	ResolvePositionCoherent(pos_linearChange, pos_angularChange);
//}
//
//void Contact3::ResolveVelocityCoherent(Vector3 linearChange[2], Vector3 angularChange[2])
//{
//	// apply velocity change
//	Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//	Vector3 impulseWorld = Vector3::ZERO;
//	if (m_friction == 0.f)
//		impulseWorld = ComputeWorldImpulse();
//	else
//	{
//		impulseWorld = ComputeWorldImpulseFriction();
//		TODO("review the system to see why i have to multiply here to make impulse work for plane collisions particularly");
//	}
//
//	if (rigid1 != nullptr && !rigid1->IsEntityStatic() && !rigid1->IsEntityKinematic())
//	{
//		Vector3 linear = impulseWorld * rigid1->GetMassData3().m_invMass;
//
//		TODO("Expose this adjuster as input parameter");
//		float torqueAdjust = (m_friction == 0.f) ? 50000.f : 1.f;			
//		Vector3 torque = m_relativePosWorld[0].Cross(impulseWorld) * torqueAdjust;
//		Vector3 rotation = rigid1->m_inverseInertiaTensorWorld * torque;
//
//		// apply change 
//		rigid1->IncrementVelocity(linear);
//		rigid1->IncrementAngularVelocity(rotation);
//
//		linearChange[0] = linear;
//		angularChange[0] = rotation;
//	}
//	else
//	{
//		linearChange[0] = Vector3::ZERO;
//		angularChange[0] = Vector3::ZERO;
//	}
//
//	Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e2);
//	if (rigid2 != nullptr && !rigid2->IsEntityStatic() && !rigid2->IsEntityKinematic())
//	{
//		impulseWorld *= -1.f;
//
//		Vector3 linear = impulseWorld * rigid2->GetMassData3().m_invMass;
//
//		Vector3 torque = m_relativePosWorld[1].Cross(impulseWorld);
//		Vector3 rotation = rigid2->m_inverseInertiaTensorWorld * torque;
//
//		rigid2->IncrementVelocity(linear);
//		rigid2->IncrementAngularVelocity(rotation);
//
//		linearChange[1] = linear;
//		angularChange[1] = rotation;
//	}
//	else
//	{
//		linearChange[1] = Vector3::ZERO;
//		angularChange[1] = Vector3::ZERO;
//	}
//}
//
//void Contact3::ResolvePositionCoherent(Vector3 linearChange[2], Vector3 angularChange[2])
//{
//	TODO("Can I NOT assume rigidbody here?");
//	Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//	Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e2);
//
//	// apply position change
//	float angularInertia[2]; float linearInertia[2];
//	float angularMove[2];	 float linearMove[2];
//
//	SolveNonlinearProjection(angularInertia, linearInertia, angularMove, linearMove);
//
//	// if static, both change arrays have Vector3::ZERO, so we do not need to consider static/kinematic on purpose
//	if (rigid1 != nullptr)
//	{
//		const Vector3& pos1 = rigid1->GetEntityCenter();
//
//		// linear
//		Vector3 translation = m_normal * linearMove[0];
//		rigid1->SetEntityCenter(pos1 + translation);	// linear change: may want to store this
//
//		// angular
//		Vector3 impulsiveTorque = m_relativePosWorld[0].Cross(m_normal);
//		Vector3 impulsePerMove = rigid1->m_inverseInertiaTensorWorld * impulsiveTorque;
//		Vector3 rotation = Vector3::ZERO;
//		if (angularInertia[0] != 0.f)
//		{
//			Vector3 rotationPerMove = impulsePerMove * (1.f / angularInertia[0]);
//			rotation = rotationPerMove * angularMove[0];		// angular change
//		}
//		Quaternion q = rigid1->GetQuaternion();
//		q.AddScaledVector(rotation, 1.f);
//		rigid1->SetQuaternion(q);		// auto normalized
//
//		linearChange[0] = translation;
//		angularChange[0] = rotation;
//
//		if (!rigid1->IsAwake()) 
//			rigid1->CacheData();
//	}
//
//	if (rigid2 != nullptr)
//	{
//		const Vector3& pos2 = rigid2->GetEntityCenter();
//
//		// linear
//		Vector3 translation = m_normal * linearMove[1];
//		rigid2->SetEntityCenter(pos2 + translation);
//
//		// angular
//		Vector3 impulsiveTorque = m_relativePosWorld[1].Cross(m_normal);
//		Vector3 impulsePerMove = rigid2->m_inverseInertiaTensorWorld * impulsiveTorque;
//		Vector3 rotation = Vector3::ZERO;
//		if (angularInertia[1] != 0.f)
//		{
//			Vector3 rotationPerMove = impulsePerMove * (1.f / angularInertia[1]);
//			rotation = rotationPerMove * angularMove[1];		// angular change
//		}
//		Quaternion q = rigid2->GetQuaternion();
//		q.AddScaledVector(rotation, 1.f);
//		rigid2->SetQuaternion(q);		// auto normalized
//
//		linearChange[1] = translation;
//		angularChange[1] = rotation;
//
//		if (!rigid2->IsAwake()) 
//			rigid2->CacheData();
//	}
//}
//
//void Contact3::RF_ResolveVelocityCoherent(Vector3 linearChange[2], Vector3 angularChange[2])
//{
//	Vector3 impulseContact;
//
//	if (!IsFrictional())
//		// compute impulse in contact coordinate
//		impulseContact = RF_ComputeFrictionlessImpulse();
//	else
//		impulseContact = RF_ComputeFrictionalImpulse();
//
//	// convert to world coordinate
//	Vector3 impulse = m_toWorld * impulseContact;
//
//	// linear and angular change
//	Vector3 impulsiveTorque = m_relativePosWorld[0].Cross(impulse);
//	angularChange[0] = m_e1->GetIITWorld() * impulsiveTorque;
//	linearChange[0].ToDefault();
//	linearChange[0] += impulse * m_e1->m_massData.m_invMass;
//
//	// apply the change
//	m_e1->m_linearVelocity += linearChange[0];
//	m_e1->SetAngularVelocity(m_e1->GetAngularVelocity() + angularChange[0]);
//
//	if (m_e2 != nullptr)
//	{
//		impulsiveTorque = impulse.Cross(m_relativePosWorld[1]);
//		angularChange[1] = m_e2->GetIITWorld() * impulsiveTorque;
//		linearChange[1].ToDefault();
//		linearChange[1] += impulse * -m_e2->m_massData.m_invMass;
//
//		m_e2->m_linearVelocity += linearChange[1];
//		m_e2->SetAngularVelocity(m_e2->GetAngularVelocity() + angularChange[1]);
//	}
//}
//
//void Contact3::RF_ResolvePositionCoherent(Vector3 linearChange[2], Vector3 angularChange[2])
//{
//	const float angularLimit = 0.2f;
//	float angularMove[2];
//	float linearMove[2];
//
//	float totalInertia = 0;
//	float linearInertia[2];
//	float angularInertia[2]; 
//
//	// i == 0
//	Matrix33 iit = m_e1->GetIITWorld();
//
//	Vector3 angularInertiaWorld = m_relativePosWorld[0].Cross(m_normal);
//	angularInertiaWorld = iit * angularInertiaWorld;
//	angularInertiaWorld = angularInertiaWorld.Cross(m_relativePosWorld[0]);
//
//	angularInertia[0] = DotProduct(angularInertiaWorld, m_normal);
//	linearInertia[0] = m_e1->m_massData.m_invMass;
//	totalInertia += linearInertia[0] + angularInertia[0];
//
//	// i == 1
//	iit = m_e2->GetIITWorld();
//
//	angularInertiaWorld = m_relativePosWorld[1].Cross(m_normal);
//	angularInertiaWorld = iit * angularInertiaWorld;
//	angularInertiaWorld = angularInertiaWorld.Cross(m_relativePosWorld[1]);
//
//	angularInertia[1] = DotProduct(angularInertiaWorld, m_normal);
//	linearInertia[1] = m_e2->m_massData.m_invMass;
//	totalInertia += linearInertia[1] + angularInertia[1];
//
//	// i == 0
//	float sign = 1.f;
//	angularMove[0] = sign * m_penetration * (angularInertia[0] / totalInertia);
//	linearMove[0] = sign * m_penetration * (linearInertia[0] / totalInertia);
//
//	Vector3 projection = m_relativePosWorld[0];
//	projection += (m_normal * -DotProduct(m_relativePosWorld[0], m_normal));
//
//	float maxMagnitude = angularLimit * projection.GetLength();
//
//	if (angularMove[0] < -maxMagnitude)
//	{
//		float totalMove = angularMove[0] + linearMove[0];
//		angularMove[0] = -maxMagnitude;
//		linearMove[0] = totalMove - angularMove[0];
//	}
//	else if (angularMove[0] > maxMagnitude)
//	{
//		float totalMove = angularMove[0] + linearMove[0];
//		angularMove[0] = maxMagnitude;
//		linearMove[0] = totalMove - angularMove[0];
//	}
//
//	if (angularMove[0] == 0)
//		angularChange[0].ToDefault();
//	else 
//	{
//		Vector3 targetAngularDirection = m_relativePosWorld[0].Cross(m_normal);
//
//		Matrix33 iit_t;
//		iit_t = m_e1->GetIITWorld();
//
//		angularChange[0] = iit_t * targetAngularDirection * (angularMove[0] / angularInertia[0]);
//	}
//
//	linearChange[0] = m_normal * linearMove[0];
//
//	Vector3 pos;
//	pos = m_e1->GetEntityCenter();
//	pos += (m_normal * linearMove[0]);
//	m_e1->SetEntityCenter(pos);
//
//	Quaternion q;
//	q = m_e1->GetQuaternion();
//	q.AddScaledVector(angularChange[0], 1.f);
//	m_e1->SetQuaternion(q);
//
//	// awake system
//	if (!m_e1->IsAwake())
//		m_e1->CacheData();
//
//	// i == 1
//	sign = -1.f;
//	angularMove[1] = sign * m_penetration * (angularInertia[1] / totalInertia);
//	linearMove[1] = sign * m_penetration * (linearInertia[1] / totalInertia);
//
//	projection = m_relativePosWorld[1];
//	projection += (m_normal * -DotProduct(m_relativePosWorld[1], m_normal));
//
//	maxMagnitude = angularLimit * projection.GetLength();
//
//	if (angularMove[1] < -maxMagnitude)
//	{
//		float totalMove = angularMove[1] + linearMove[1];
//		angularMove[1] = -maxMagnitude;
//		linearMove[1] = totalMove - angularMove[1];
//	}
//	else if (angularMove[1] > maxMagnitude)
//	{
//		float totalMove = angularMove[1] + linearMove[1];
//		angularMove[1] = maxMagnitude;
//		linearMove[1] = totalMove - angularMove[1];
//	}
//
//	if (angularMove[1] == 0)
//		angularChange[1].ToDefault();
//	else 
//	{
//		Vector3 targetAngularDirection = m_relativePosWorld[1].Cross(m_normal);
//
//		Matrix33 iit_t;
//		iit_t = m_e2->GetIITWorld();
//
//		angularChange[1] = iit_t * targetAngularDirection * (angularMove[1] / angularInertia[1]);
//	}
//
//	linearChange[1] = m_normal * linearMove[1];
//
//	pos = m_e2->GetEntityCenter();
//	pos += (m_normal * linearMove[1]);
//	m_e2->SetEntityCenter(pos);
//
//	q = m_e2->GetQuaternion();
//	q.AddScaledVector(angularChange[1], 1.f);
//	m_e2->SetQuaternion(q);
//
//	// wake system
//	if (!m_e2->IsAwake())
//		m_e2->CacheData();
//}
//
//void Contact3::PrepareInternal(float deltaTime)
//{
//	if (m_e1 == nullptr)
//	{
//		ASSERT_RECOVERABLE(false, "First entity of collision pair should not be nullptr even before swapping");
//		SwapEntities();
//	}
//	ASSERT_OR_DIE(m_e1 != nullptr, "Swapped body should not be null");
//
//	// contact matrix
//	MakeToWorld();
//
//	m_relativePosWorld[0] = m_point - m_e1->GetEntityCenter();
//	if (m_e2 != nullptr)
//		m_relativePosWorld[1] = m_point - m_e2->GetEntityCenter();
//
//	// If entity is static/kinematic, it will have NO contribution to closing vel
//	// because their angular/linear velocity will 0.
//	// Hence we do not check for static/kinematic here
//	m_closingVel = ComputeContactVelocity(0, m_e1, deltaTime);
//	if (m_e2 != nullptr)
//		m_closingVel -= ComputeContactVelocity(1, m_e2, deltaTime);
//
//	// desired change in vel as resolving coherent contacts
//	ComputeDesiredVelDeltaCoherent(deltaTime);
//	//ComputeDesiredVelDeltaResting(deltaTime);
//}
//
//void Contact3::SwapEntities()
//{
//	m_normal *= -1.f;
//
//	Entity3* temp = m_e1;
//	m_e1 = m_e2;
//	m_e2 = temp;
//
//	// swap feature
//	eContactFeature tempFeature = m_f1;
//	m_f1 = m_f2;
//	m_f2 = tempFeature;
//}
//
//Vector3 Contact3::ComputeContactVelocity(int idx, Entity3* ent, float deltaTime)
//{
//	//Rigidbody3* rigid = static_cast<Rigidbody3*>(ent);
//	Vector3 velocity = ent->GetAngularVelocity().Cross(m_relativePosWorld[idx]);
//	velocity += ent->GetLinearVelocity();
//
//	// contact coord basis is orthonormal, so we use transpose as inverse 
//	const Matrix33& toContact = m_toWorld.GetTranspose();
//	Vector3 contactVelocity = toContact * velocity;	// to contact coord
//
//	// TODO: restore when validating friction pipeline
//	Vector3 accVelocity = ent->GetLastFrameLinearAcc() * deltaTime;
//	accVelocity = toContact * accVelocity;
//	accVelocity.x = 0.f;					// ignore acceleration along local normal direction
//
//	// if there is enough friction this will be removed during vel resolution
//	contactVelocity += accVelocity;
//
//	return contactVelocity;
//}
//
//// DEPRECATED
//void Contact3::ComputeDesiredVelDeltaCoherent(float deltaTime)
//{
//	const static float velocityLimit = .25f;
//
//	// Calculate the acceleration induced velocity accumulated this frame
//	float velocityFromAcc = 0;
//
//	if (m_e1->IsAwake())
//		velocityFromAcc += DotProduct(m_e1->GetLastFrameLinearAcc() * deltaTime, m_normal);
//
//	if (m_e2 && m_e2->IsAwake())
//		velocityFromAcc -= DotProduct(m_e2->GetLastFrameLinearAcc() * deltaTime, m_normal);
//
//	// If the velocity is very slow, limit the restitution
//	float thisRestitution = m_restitution;
//	if (abs(m_closingVel.x) < velocityLimit)
//		thisRestitution = 0.f;
//
//	// Combine the bounce velocity with the removed
//	// acceleration velocity.
//	m_desiredVelDelta = -m_closingVel.x - thisRestitution * (m_closingVel.x - velocityFromAcc);
//}
//
//// built on ComputeDesiredVelDeltaCoherent()
//void Contact3::ComputeDesiredVelDeltaResting(float deltaTime)
//{
//	const static float velLimit = .25f;
//
//	float velFromAcc = 0.f;
//
//	// technically i do not care static/kinematic here...
//	// because last frame acc for those rigid bodies will be 0
//	Rigidbody3* r1 = static_cast<Rigidbody3*>(m_e1);
//	if (r1 != nullptr && r1->IsAwake() && !r1->IsEntityStatic() && !r1->IsEntityKinematic())
//	//if (r1 != nullptr && r1->IsAwake())
//		velFromAcc += DotProduct(r1->m_lastFrameLinearAcc, m_normal) * deltaTime;
//	
//	Rigidbody3* r2 = static_cast<Rigidbody3*>(m_e2);
//	if (r2 != nullptr && r2->IsAwake() && !r2->IsEntityStatic() && !r2->IsEntityKinematic())
//	//if (r2 != nullptr && r2->IsAwake())
//		velFromAcc -= DotProduct(r2->m_lastFrameLinearAcc, m_normal) * deltaTime;
//
//	float r = m_restitution;
//	if (m_closingVel.GetLength() < velLimit)
//		r = 0.f;
//
//	m_desiredVelDelta = -m_closingVel.x - r * (m_closingVel.x - velFromAcc);
//}
//
//void Contact3::WakeUp()
//{
//	if (m_e2 == nullptr)
//		return;
//
//	// only rigid body binds up with sleep system
//	//Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//	//Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e2);
//
//	//bool firstAwake = rigid1->IsAwake();
//	//bool secondAwake = rigid2->IsAwake();
//	bool firstAwake = m_e1->IsAwake();
//	bool secondAwake = m_e2->IsAwake();
//
//	bool processing = firstAwake ^ secondAwake;
//
//	// there is one rigid body awake while the other is not, need to process
//	if (processing)
//	{
//		if (firstAwake)
//			m_e2->SetAwake(true);
//		else
//			m_e1->SetAwake(true);
//	}
//}
//
//void Contact3::SolveNonlinearProjection(		
//	float angularInertia[2], float linearInertia[2],
//	float angularMove[2], float linearMove[2])
//{
//	float totalInertia = 0.f;
//	Rigidbody3* rigid1 = dynamic_cast<Rigidbody3*>(m_e1);
//	Rigidbody3* rigid2 = dynamic_cast<Rigidbody3*>(m_e2);
//
//	if (rigid1 != nullptr && !rigid1->IsEntityStatic() && !rigid1->IsEntityKinematic())
//	{
//		const Matrix33& iitWorld = rigid1->m_inverseInertiaTensorWorld;
//
//		Vector3 angInertiaWorld = m_relativePosWorld[0].Cross(m_normal);
//		angInertiaWorld = iitWorld * angInertiaWorld;
//		angInertiaWorld = angInertiaWorld.Cross(m_relativePosWorld[0]);
//
//		angularInertia[0] = DotProduct(angInertiaWorld, m_normal);
//		linearInertia[0] = rigid1->GetMassData3().m_invMass;
//
//		totalInertia += (angularInertia[0] + linearInertia[0]);
//	}
//	if (rigid2 != nullptr && !rigid2->IsEntityStatic() && !rigid2->IsEntityKinematic())
//	{
//		const Matrix33& iitWorld = rigid2->m_inverseInertiaTensorWorld;
//
//		Vector3 angInertiaWorld = m_relativePosWorld[1].Cross(m_normal);
//		angInertiaWorld = iitWorld * angInertiaWorld;
//		angInertiaWorld = angInertiaWorld.Cross(m_relativePosWorld[1]);
//
//		angularInertia[1] = DotProduct(angInertiaWorld, m_normal);
//		linearInertia[1] = rigid2->GetMassData3().m_invMass;
//
//		totalInertia += (angularInertia[1] + linearInertia[1]);
//	}
//
//	float contact_ii = 1.f / totalInertia;
//	if (!rigid1->IsEntityStatic() && !rigid1->IsEntityKinematic())
//	{
//		linearMove[0] = m_penetration * linearInertia[0] * contact_ii;
//		angularMove[0] = m_penetration * angularInertia[0] * contact_ii;
//	}
//	else
//	{
//		linearMove[0] = 0.f;
//		angularMove[0] = 0.f;
//	}
//
//	if (!rigid2->IsEntityStatic() && !rigid2->IsEntityKinematic())
//	{
//		linearMove[1] = -m_penetration * linearInertia[1] * contact_ii;
//		angularMove[1] = -m_penetration * angularInertia[1] * contact_ii;
//	}
//	else
//	{
//		linearMove[1] = 0.f;
//		angularMove[1] = 0.f;
//	}
//
//	// limit angular movement 
//	// does not affect static objects - move values will be 0
//	const float angularLimit = .2f;
//	float limit1 = angularLimit * m_relativePosWorld[0].GetLength();
//	if (abs(angularMove[0]) > limit1)
//	{
//		float totalMove1 = linearMove[0] + angularMove[0];
//
//		if (angularMove[0] >= 0.f)
//			angularMove[0] = limit1;
//		else
//			angularMove[0] = -limit1;
//
//		linearMove[0] = totalMove1 - angularMove[0];
//	}
//
//	float limit2 = angularLimit * m_relativePosWorld[1].GetLength();
//	if (abs(angularMove[1]) > limit2)
//	{
//		float totalMove2 = linearMove[1] + angularMove[1];
//
//		if (angularMove[1] >= 0.f)
//			angularMove[1] = limit2;
//		else
//			angularMove[1] = -limit2;
//
//		linearMove[1] = totalMove2 - angularMove[1];
//	}
//}
//
//float Contact3::ComputeSeparatingVelocity() const
//{
//	Vector3 e1_vel = m_e1->GetLinearVelocity();
//	Vector3 e2_vel;
//	if (m_e2 != nullptr)
//		e2_vel = m_e2->GetLinearVelocity();
//	Vector3 rel_vel = e1_vel - e2_vel;
//	return DotProduct(rel_vel, m_normal);
//}
//
//void Contact3::ResolveVelocity(float deltaTime)
//{
//	float separate = ComputeSeparatingVelocity();
//	// separating or stationary
//	if (separate > 0)
//		return;
//
//	// Calculate the new separating velocity and delta velocity
//	float new_separate = -separate * m_restitution;
//
//	// resolve resting contact 
//	// Check the velocity build-up due to acceleration only.
//	Vector3 e1_acc = m_e1->GetLinearAcceleration();
//	Vector3 e2_acc = Vector3::ZERO;
//	if (m_e2 != nullptr)
//		e2_acc = m_e2->GetLinearAcceleration();
//	Vector3 rel_acc = e1_acc - e2_acc;
//	float rel_vel = DotProduct(rel_acc, m_normal) * deltaTime;
//	// If we�ve got a closing velocity due to acceleration build-up,
//	// remove it from the new separating velocity.
//	if (rel_vel < 0.f)
//	{
//		new_separate += m_restitution * rel_vel;
//		// Make sure we haven�t removed more than was there to remove.
//		if (new_separate < 0) 
//			new_separate = 0;
//	}
//
//	float delta_vel = new_separate - separate;
//
//	// We apply the change in velocity to each object in proportion to
//	// its inverse mass (i.e., those with lower inverse mass [higher
//	// actual mass] get less change in velocity).
//	float e1_inv_mass = m_e1->GetMassData3().m_invMass;
//	float e2_inv_mass = 0.f;
//	if (m_e2 != nullptr)
//		e2_inv_mass = m_e2->GetMassData3().m_invMass;
//	float total_inv_mass = e1_inv_mass + e2_inv_mass;
//	// If all particles have infinite mass, then impulses have no effect.
//	if (total_inv_mass <= 0.f) 
//		return;
//	// Calculate the impulse to apply.
//	float impulse_amount = delta_vel / total_inv_mass;
//	// Find the amount of impulse per unit of inverse mass.
//	Vector3 impulse_per_inv_mass = m_normal * impulse_amount;
//
//	// Apply impulses: they are applied in the direction of the contact,
//	// and are proportional to the inverse mass.
//	if (!m_e1->IsEntityStatic() && !m_e1->IsEntityKinematic())
//		m_e1->SetLinearVelocity(m_e1->GetLinearVelocity() +
//			impulse_per_inv_mass * m_e1->GetMassData3().m_invMass);
//	if (m_e2 != nullptr)
//	{
//		if (!m_e2->IsEntityStatic() && !m_e2->IsEntityKinematic())
//			// Particle 1 goes in the opposite direction.
//			m_e2->SetLinearVelocity(m_e2->GetLinearVelocity() +
//				impulse_per_inv_mass * (-m_e2->GetMassData3().m_invMass));
//	}
//}
//
//void Contact3::ResolvePenetration(float)
//{
//	// If we don�t have any penetration, skip this step.
//	if (m_penetration <= 0) 
//		return;
//	// The movement of each object is based on its inverse mass, so
//	// total that.
//	float e1_inv_mass = m_e1->GetMassData3().m_invMass;
//	float e2_inv_mass = 0.f;
//	if (m_e2 != nullptr)
//		e2_inv_mass = m_e2->GetMassData3().m_invMass;
//	float total_inv_mass = e1_inv_mass + e2_inv_mass;
//	// If all particles have infinite mass, then we do nothing.
//	if (total_inv_mass <= 0) 
//		return;
//	// Find the amount of penetration resolution per unit of inverse mass.
//	Vector3 move_per_inv_mass = m_normal * (m_penetration / total_inv_mass);
//	// Apply the penetration resolution.
//	if (!m_e1->IsEntityStatic())
//		m_e1->SetEntityCenter(m_e1->GetEntityCenter() +
//			move_per_inv_mass * m_e1->GetMassData3().m_invMass);
//	if (m_e2 != nullptr)
//	{
//		if (!m_e2->IsEntityStatic())
//			m_e2->SetEntityCenter(m_e2->GetEntityCenter() +
//				move_per_inv_mass * (-m_e2->GetMassData3().m_invMass));
//	}
//}
//
//void CollisionData3::ClearCoherent()
//{
//	for (std::vector<Contact3>::size_type idx = 0; idx < m_contacts.size(); ++idx)
//	{
//		if (m_contacts[idx].m_penetration < COHERENT_THRESHOLD)
//		{
//			std::vector<Contact3>::iterator it = m_contacts.begin() + idx;
//			m_contacts.erase(it);
//			idx--;
//		}
//	}
//}
//
//
//bool CollisionData3::HasAndUpdateContact(const Contact3& contact)
//{
//	if (contact.DoesFeatureMatter())
//	{
//		bool found = false;
//
//		for (Contact3& c : m_contacts)
//		{
//			// entities match 
//			bool ent1Match = (c.m_e1 == contact.m_e1);
//			bool ent2Match = (c.m_e2 == contact.m_e2);
//			
//			// type match
//			bool typeMatch = (c.m_type == contact.m_type);
//
//			// feature match
//			bool f1Match = (c.m_f1 == contact.m_f1);
//			bool f2Match = (c.m_f2 == contact.m_f2);
//
//			if (ent1Match && ent2Match && typeMatch && f1Match && f2Match)
//			{
//				// found same contact
//				found = true;
//
//				// update the contact instead of adding it
//				c = contact;
//
//				break;
//			}
//		}
//
//		return found;
//	}
//	else
//	{
//		bool found = false;
//
//		// if the contact is not feature-important, we only compare if the entities match 
//		for (Contact3& c : m_contacts)
//		{
//			// entities match 
//			bool ent1Match = (c.m_e1 == contact.m_e1);
//			bool ent2Match = (c.m_e2 == contact.m_e2);
//
//			if (ent1Match && ent2Match)
//			{
//				// found the same contact 
//				found = true;
//
//				// update it
//				c = contact;
//
//				// and quit
//				break;
//			}
//		}
//
//		return found;
//	}
//}
//
//
//const Vector3 CollisionDetector::ISA = Vector3(-INFINITY);
//
//CollisionDetector::CollisionDetector()
//{
//
//}
//
//CollisionDetector::~CollisionDetector()
//{
//
//}
//
//bool CollisionDetector::Sphere3VsSphere3Core(const Sphere3& s1, const Sphere3& s2, Contact3& contact)
//{
//	const Vector3& s1Pos = s1.m_center;
//	const Vector3& s2Pos = s2.m_center;
//	const float& s1Rad = s1.m_radius;
//	const float& s2Rad = s2.m_radius;
//
//	Vector3 midLine = s1Pos - s2Pos;
//	float length = midLine.GetLength();
//
//	// allow pessimistic collision detection
//	TODO("Need to test this more with ALL cases, i.e. resting");
//	//float pessimistic = 0.f;
//	if (length <= 0.f || length >= (s1Rad + s2Rad))
//	{
//		// if mid line length is invalid
//		// or, larger than radius threshold, return directly
//		return false;
//	}
//
//	// get normal
//	Vector3 normal = midLine.GetNormalized();
//	//Vector3 point = s2Pos + midLine * 0.5f;
//	Vector3 point = s2Pos + normal * s2Rad;
//	float penetration = s1Rad + s2Rad - length;
//
//	TODO("Hook friction with physics material with both entities");
//	// for the contact type we do not care, so it is by default NO_CARE
//	// also in this case we do not care which feature the contact comes from, so leave it FEATURE_NO_CARE
//	Contact3 theContact = Contact3(s1.GetEntity(), s2.GetEntity(), normal, point, penetration, 1.f, .1f);	
//	contact = theContact;
//
//	return true;
//}
//
//uint CollisionDetector::Sphere3VsSphere3Single(const Sphere3& s1, const Sphere3& s2, CollisionData3* data)
//{
//	if (data->m_contacts.size() > data->m_maxContacts)
//	{
//		// no contacts amount left, return directly
//		return 0;
//	}
//
//	Contact3 theContact;
//	bool contactGenerated = Sphere3VsSphere3Core(s1, s2, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::Sphere3VsSphere3Coherent(const Sphere3& s1, const Sphere3& s2, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	Contact3 theContact;
//	bool contactGenerated = Sphere3VsSphere3Core(s1, s2, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	bool existed = data->HasAndUpdateContact(theContact);
//
//	if (!existed)
//		data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//bool CollisionDetector::Sphere3VsPlane3Core(const Sphere3& sph, const Plane& pl, Contact3& contact)
//{
//	const float& sphereRad = sph.GetRadius();
//	const Vector3& spherePos = sph.GetCenter();
//	const Vector3& planeNormal = pl.GetNormal().GetNormalized();			// guarantee to be normalized
//	float sphereToOriginAlongPlaneDir = DotProduct(planeNormal, spherePos);
//	float signedDistToPlane = sphereToOriginAlongPlaneDir - pl.GetOffset();
//	TODO("All core updates should allow a penetration value smaller than threshold, see line 11 CollisionDetection.hpp");
//	if (abs(signedDistToPlane) >= sphereRad)
//		return false;
//
//	// we are certain that there is some contact between sphere and plane
//
//	// check which side of plane we are on
//	Vector3 usedNormal = planeNormal;
//	float penetration = -signedDistToPlane;
//	if (signedDistToPlane < 0)
//	{
//		usedNormal *= -1.f;
//		penetration = -penetration;
//	}
//	penetration += sph.GetRadius();
//
//	// no feature hence no feature identity
//	Vector3 contactPoint = spherePos - planeNormal * signedDistToPlane;
//	TODO("If we are to add frcition, we need to multiply frcition impulse by 2 for some reason, see comment at line 270");
//	Contact3 theContact = Contact3(sph.GetEntity(), pl.GetEntity(), usedNormal, contactPoint, penetration, 1.f, 0.f);
//	contact = theContact;
//
//	return true;
//}
//
//uint CollisionDetector::Sphere3VsPlane3Single(const Sphere3& sph, const Plane& pl, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	Contact3 theContact;
//	bool contactGenerated = Sphere3VsPlane3Core(sph, pl, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::Sphere3VsPlane3Coherent(const Sphere3& sph, const Plane& pl, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	// core of intersection test: sphere vs plane
//	Contact3 theContact;
//	bool contactGenerated = Sphere3VsPlane3Core(sph, pl, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	bool existed = data->HasAndUpdateContact(theContact);
//
//	if (!existed)
//		data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::Sphere3VsPlane3Continuous(const Sphere3& sph, const Plane& pl, const Vector3& v, float& t, Vector3& hit)
//{
//	// sph to plane distance
//	float dist = DotProduct(pl.m_normal, sph.m_center);
//	dist -= pl.m_offset;
//
//	if (abs(dist) <= sph.m_radius)
//	{
//		// sphere is already overlapping plane.
//		// set TOI to 0 and impact point to sphere center
//		t = 0.f;
//		hit = sph.m_center;
//		return 1;
//	}
//	else
//	{
//		float denom = DotProduct(pl.m_normal, v);
//		if (denom * dist >= 0.f)
//			// sphere moving parallel to or away from the plane
//			return 0;
//		else
//		{
//			// sphere moving towards the plane
//			// use +r if sphere in front of plane, -r otherwise
//			float r = dist > 0.f ? sph.m_radius : -sph.m_radius;
//			t = (r - dist) / denom;
//			hit = sph.m_center + v * t - pl.m_normal * r;
//			return 1;
//		}
//	}
//}
//
//uint CollisionDetector::Sphere3VsAABB3(const Sphere3& sph, const AABB3& aabb3, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//	{
//		// no contacts amount left, return directly
//		return 0;
//	}
//
//	// transform sphere to aabb3 coord
//	// get transform of aabb3 first and transform the basis from that
//	const Transform& aabb3Transform = aabb3.GetEntity()->GetEntityTransform();
//
//	float sphereRad = sph.GetRadius();
//	Vector3 worldSpherePos = sph.GetCenter();
//	Vector3 localSpherePos = Transform::TransformWorldToLocalPosOrthogonal(worldSpherePos, aabb3Transform);
//	Vector3 aabb3HalfDim = aabb3.GetDimensions() / 2.f;
//
//	// early out in aabb3 coord
//	bool xtest = (abs(localSpherePos.x) - sphereRad) > aabb3HalfDim.x;
//	bool ytest = (abs(localSpherePos.y) - sphereRad) > aabb3HalfDim.y;
//	bool ztest = (abs(localSpherePos.z) - sphereRad) > aabb3HalfDim.z;
//	if (xtest || ytest || ztest)
//	{
//		return 0;
//	}
//
//	Vector3 closestPointLocal = Vector3::ZERO;
//	float dist;
//
//	// clamp x
//	dist = localSpherePos.x;
//	if (dist > aabb3HalfDim.x) dist = aabb3HalfDim.x;
//	if (dist < -aabb3HalfDim.x) dist = -aabb3HalfDim.x;
//	closestPointLocal.x = dist;
//
//	// clamp y
//	dist = localSpherePos.y;
//	if (dist > aabb3HalfDim.y) dist = aabb3HalfDim.y;
//	if (dist < -aabb3HalfDim.y) dist = -aabb3HalfDim.y;
//	closestPointLocal.y = dist;
//
//	// clamp z
//	dist = localSpherePos.z;
//	if (dist > aabb3HalfDim.z) dist = aabb3HalfDim.z;
//	if (dist < -aabb3HalfDim.z) dist = -aabb3HalfDim.z;
//	closestPointLocal.z = dist;
//
//	// check if we are in contact
//	dist = (closestPointLocal - localSpherePos).GetLengthSquared();
//	if (dist > sphereRad * sphereRad) return 0;
//
//	// at this point we get the contact point in the local space
//	// we need to transform it back to world space
//	Vector3 closestPointWorld = Transform::TransformLocalToWorldPos(closestPointLocal, aabb3Transform);
//
//	Vector3 usedNormal = (worldSpherePos - closestPointWorld).GetNormalized();
//	Vector3 contactPoint = closestPointWorld;
//	float penetration = sphereRad - sqrtf(dist);
//	Contact3 theContact = Contact3(sph.GetEntity(), aabb3.GetEntity(),
//		usedNormal, contactPoint, penetration);
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::AABB3VsAABB3Coherent(const AABB3&, const AABB3&, CollisionData3*)
//{
//	/////////////////////////////////////// NON-COHERENCE PROCESSING ///////////////////////////////////////
//	//uint succeed = NonCoherentProcessingAABB3VsAABB3(aabb3_1, aabb3_2, data);
//	/////////////////////////////////////// END ///////////////////////////////////////
//
//	/////////////////////////////////////// COHERENCE PROCESSING ///////////////////////////////////////
//	//uint succeed = CoherentProcessingAABB3VsAABB3(aabb3_1, aabb3_2, data);	// comment out one or the other for coherency
//	/////////////////////////////////////// END ///////////////////////////////////////
//
//	return 0;
//}
//
//uint CollisionDetector::AABB3VsAABB3Single(const AABB3& aabb3_1, const AABB3& aabb3_2, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//	{
//		// no contacts amount left, return directly
//		return 0;
//	}
//
//	Vector3 overlap_axis;
//	float penetration;
//	bool overlapped = AABB3VsAABB3Intersection(aabb3_1, aabb3_2, overlap_axis, penetration);
//
//	if (!overlapped)
//		return 0;
//
//	// get normal
//	Vector3 second_to_first_aabb3_center = aabb3_1.GetCenter() - aabb3_2.GetCenter();
//	overlap_axis.NormalizeAndGetLength();
//	float extend = DotProduct(second_to_first_aabb3_center, overlap_axis);
//	Vector3 usedNormal = (overlap_axis * extend).GetNormalized();
//
//	// get contact point
//	Vector3 halfDim = aabb3_1.GetHalfDimensions();
//	Vector3 to_second_normal = -usedNormal;
//	float abs_half_dim = abs(DotProduct(halfDim, to_second_normal));
//	Vector3 first_half_dim_along_normal = to_second_normal * abs_half_dim;
//	Vector3 to_second_overlap = to_second_normal * penetration;
//	Vector3 to_contact_point = first_half_dim_along_normal - to_second_overlap;
//	Vector3 contactPoint = aabb3_1.GetCenter() + to_contact_point;
//
//	// create contact
//	Contact3 theContact = Contact3(aabb3_1.GetEntity(), aabb3_2.GetEntity(),
//		usedNormal, contactPoint, penetration);
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//bool CollisionDetector::OBB3VsPlane3Core(const OBB3& obb, const Plane& pl, Contact3& contact)
//{
//	float s = DotProduct(pl.m_normal, obb.m_center) - pl.m_offset;
//	if (s > 0.f)		// above plane
//	{
//		float supp_ext = INFINITY;
//		const Vector3& pl_norm = pl.GetNormal().GetNormalized();
//		OBB3Vert res;
//		//bool overlapped = false;
//
//		float ftl_ext = obb.GetFTLExtAlong(pl_norm);
//		if (ftl_ext < supp_ext)
//		{
//			supp_ext = ftl_ext;
//			res = obb.GetFTLVert();
//		}
//
//		float fbl_ext = obb.GetFBLExtAlong(pl_norm);
//		if (fbl_ext < supp_ext)
//		{
//			supp_ext = fbl_ext;
//			res = obb.GetFBLVert();
//		}
//
//		float fbr_ext = obb.GetFBRExtAlong(pl_norm);
//		if (fbr_ext < supp_ext)
//		{
//			supp_ext = fbr_ext;
//			res = obb.GetFBRVert();
//		}
//
//		float ftr_ext = obb.GetFTRExtAlong(pl_norm);
//		if (ftr_ext < supp_ext)
//		{
//			supp_ext = ftr_ext;
//			res = obb.GetFTRVert();
//		}
//
//		float btl_ext = obb.GetBTLExtAlong(pl_norm);
//		if (btl_ext < supp_ext)
//		{
//			supp_ext = btl_ext;
//			res = obb.GetBTLVert();
//		}
//
//		float bbl_ext = obb.GetBBLExtAlong(pl_norm);
//		if (bbl_ext < supp_ext)
//		{
//			supp_ext = bbl_ext;
//			res = obb.GetBBLVert();
//		}
//
//		float bbr_ext = obb.GetBBRExtAlong(pl_norm);
//		if (bbr_ext < supp_ext)
//		{
//			supp_ext = bbr_ext;
//			res = obb.GetBBRVert();
//		}
//
//		float btr_ext = obb.GetBTRExtAlong(pl_norm);
//		if (btr_ext < supp_ext)
//		{
//			supp_ext = btr_ext;
//			res = obb.GetBTRVert();
//		}
//
//		// see if overlap: if the support point cross the plane, or no
//		if (abs(supp_ext) < s)
//			return false;
//
//		// they do overlap, compute contact data
//		float penetration = abs(supp_ext) - s;
//		Vector3 contactPt = res.m_vert + pl_norm * penetration;
//
//		contact = Contact3(obb.GetEntity(), pl.GetEntity(), pl_norm, contactPt, penetration, 0.9f);
//		contact.m_f1 = res.m_feature;
//		contact.m_type = POINT_FACE;
//		// second feature is still NO_CARE because the second entity is a plane
//
//		return true;
//	}
//	else     // below the plane
//	{
//		float supp_ext = -INFINITY;
//		const Vector3& pl_norm = pl.GetNormal().GetNormalized();
//		OBB3Vert res;
//
//		float ftl_ext = obb.GetFTLExtAlong(pl_norm);
//		if (ftl_ext > supp_ext)
//		{
//			supp_ext = ftl_ext;
//			res = obb.GetFTLVert();
//		}
//
//		float fbl_ext = obb.GetFBLExtAlong(pl_norm);
//		if (fbl_ext > supp_ext)
//		{
//			supp_ext = fbl_ext;
//			res = obb.GetFBLVert();
//		}
//
//		float fbr_ext = obb.GetFBRExtAlong(pl_norm);
//		if (fbr_ext > supp_ext)
//		{
//			supp_ext = fbr_ext;
//			res = obb.GetFBRVert();
//		}
//
//		float ftr_ext = obb.GetFTRExtAlong(pl_norm);
//		if (ftr_ext > supp_ext)
//		{
//			supp_ext = ftr_ext;
//			res = obb.GetFTRVert();
//		}
//
//		float btl_ext = obb.GetBTLExtAlong(pl_norm);
//		if (btl_ext > supp_ext)
//		{
//			supp_ext = btl_ext;
//			res = obb.GetBTLVert();
//		}
//
//		float bbl_ext = obb.GetBBLExtAlong(pl_norm);
//		if (bbl_ext > supp_ext)
//		{
//			supp_ext = bbl_ext;
//			res = obb.GetBBLVert();
//		}
//
//		float bbr_ext = obb.GetBBRExtAlong(pl_norm);
//		if (bbr_ext > supp_ext)
//		{
//			supp_ext = bbr_ext;
//			res = obb.GetBBRVert();
//		}
//
//		float btr_ext = obb.GetBTRExtAlong(pl_norm);
//		if (btr_ext > supp_ext)
//		{
//			supp_ext = btr_ext;
//			res = obb.GetBTRVert();
//		}
//
//		if (supp_ext < abs(s))
//			return false;
//
//		// they do overlap, compute contact data
//		float penetration = supp_ext - abs(s);
//		Vector3 usedNormal = -pl_norm;
//		Vector3 contactPt = res.m_vert + usedNormal * penetration;
//
//		contact = Contact3(obb.GetEntity(), pl.GetEntity(), usedNormal, contactPt, penetration, 0.9f);
//		contact.m_f1 = res.m_feature;
//		contact.m_type = POINT_FACE;
//		// second feature is still NO_CARE because the second entity is a plane
//
//		return true;
//	}
//
//	/*
//	float r = obb.m_halfExt.x * abs(DotProduct(pl.m_normal, obb.m_right)) +
//	obb.m_halfExt.y * abs(DotProduct(pl.m_normal, obb.m_up)) +
//	obb.m_halfExt.z * abs(DotProduct(pl.m_normal, obb.m_forward));
//	float s = DotProduct(pl.m_normal, obb.m_center) - pl.m_offset;
//
//	if (abs(s) > r)
//	return false;
//
//	float penetration = r - abs(s);		// > 0
//	Vector3 usedNormal;
//	if (s > 0)
//	usedNormal = pl.GetNormal();
//	else
//	usedNormal = -pl.GetNormal();
//	Vector3 contactPoint = obb.GetCenter() + (-usedNormal) * s;
//	Contact3 theContact = Contact3(obb.GetEntity(), pl.GetEntity(), usedNormal, contactPoint, penetration, 0.9f);
//	contact = theContact;
//
//	return true;
//	*/
//}
//
//
//uint CollisionDetector::OBB3VsPlane3Single(const OBB3& obb, const Plane& plane, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	Contact3 theContact;
//	bool contactGenerated = OBB3VsPlane3Core(obb, plane, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::OBB3VsPlane3Coherent(const OBB3& obb, const Plane& plane, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	// core of intersection test: sphere vs plane
//	Contact3 theContact;
//	bool contactGenerated = OBB3VsPlane3Core(obb, plane, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	bool existed = data->HasAndUpdateContact(theContact);
//
//	if (!existed)
//		data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//bool CollisionDetector::OBB3VsSphere3Core(const OBB3& obb, const Sphere3& sphere, Contact3& contact)
//{
//	// get obb transform
//	BoxEntity3* boxEnt = dynamic_cast<BoxEntity3*>(obb.GetEntity());
//	BoxRB3* boxRb = dynamic_cast<BoxRB3*>(obb.GetEntity());
//	Transform t;
//	if (boxEnt != nullptr)
//		t = boxEnt->GetEntityTransform();
//	else
//		t = boxRb->GetEntityTransform();
//	
//	// transform sphere to local coord of obb 
//	Vector3 center_local = Transform::WorldToLocalOrthogonal(sphere.GetCenter(), t);
//	const float& r = sphere.GetRadius();
//	Sphere3 local_sph = Sphere3(center_local, r);
//
//	// construct local aabb
//	const Vector3& halfExt = obb.GetHalfExt();
//	AABB3 local_box = AABB3(-halfExt, halfExt);
//	Vector3 aabb3HalfDim = local_box.GetDimensions() / 2.f;
//
//	// early out in aabb3 coord
//	bool xtest = (abs(center_local.x) - r) > aabb3HalfDim.x;
//	bool ytest = (abs(center_local.y) - r) > aabb3HalfDim.y;
//	bool ztest = (abs(center_local.z) - r) > aabb3HalfDim.z;
//	if (xtest || ytest || ztest)
//		return false;
//
//	Vector3 closestPointLocal = Vector3::ZERO;
//	float dist;
//
//	// clamp x
//	dist = center_local.x;
//	if (dist > aabb3HalfDim.x) dist = aabb3HalfDim.x;
//	if (dist < -aabb3HalfDim.x) dist = -aabb3HalfDim.x;
//	closestPointLocal.x = dist;
//
//	// clamp y
//	dist = center_local.y;
//	if (dist > aabb3HalfDim.y) dist = aabb3HalfDim.y;
//	if (dist < -aabb3HalfDim.y) dist = -aabb3HalfDim.y;
//	closestPointLocal.y = dist;
//
//	// clamp z
//	dist = center_local.z;
//	if (dist > aabb3HalfDim.z) dist = aabb3HalfDim.z;
//	if (dist < -aabb3HalfDim.z) dist = -aabb3HalfDim.z;
//	closestPointLocal.z = dist;
//
//	// check if we are in contact
//	dist = (closestPointLocal - center_local).GetLengthSquared();
//	if (dist > r * r) 
//		return false;
//
//	// at this point we get the contact point in the local space
//	// we need to transform it back to world space
//	Vector3 closestPointWorld = Transform::LocalToWorldPos(closestPointLocal, t);
//
//	Vector3 usedNormal = (sphere.GetCenter() - closestPointWorld).GetNormalized();
//	Vector3 contactPoint = closestPointWorld;
//	float penetration = r - sqrtf(dist);
//	Contact3 theContact = Contact3(sphere.GetEntity(), obb.GetEntity(), usedNormal, contactPoint, penetration, 1.f, 0.01f);
//	contact = theContact;
//	// contact is feature-unimportant, we do not care type, nor do we care feature id
//	
//	return true;
//}
//
//uint CollisionDetector::OBB3VsSphere3Single(const OBB3& obb, const Sphere3& sphere, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	Contact3 theContact;
//	bool contactGenerated = OBB3VsSphere3Core(obb, sphere, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::OBB3VsSphere3Coherent(const OBB3& obb, const Sphere3& sphere, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	// core of intersection test: sphere vs plane
//	Contact3 theContact;
//	bool contactGenerated = OBB3VsSphere3Core(obb, sphere, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	bool existed = data->HasAndUpdateContact(theContact);
//
//	if (!existed)
//		data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
///*
// * Check if the two obb3 intersect. No contact info returned.
// */
//bool CollisionDetector::OBB3VsOBB3Intersected(const OBB3& obb1, const OBB3& obb2)
//{
//	Vector3 obb1_local[3] = {obb1.m_right, obb1.m_up, obb1.m_forward};	// x y z
//	Vector3 obb2_local[3] = {obb2.m_right, obb2.m_up, obb2.m_forward};
//
//	// SAT, if fails, no collision
//	// 1 - rotation matrix where obb2 is expressed with obb1 coord (obb2 -> obb1)
//	float entries[9];
//	for (int i = 0; i < 3; ++i)
//	{
//		for (int j = 0; j < 3; ++j)
//		{
//			int idx = 3 * j + i;
//			entries[idx] = DotProduct(obb1_local[i], obb2_local[j]);
//		}
//	}
//	Matrix33 rotation = Matrix33(entries);
//
//	// 2 - find translation t from obb1 to obb2, and use that to translate to obb1's frame
//	Vector3 t = obb2.m_center - obb1.m_center;
//	t = Vector3(DotProduct(t, obb1.m_right), 
//		DotProduct(t, obb1.m_up),
//		DotProduct(t, obb1.m_forward));
//
//	// 3 - robust SAT
//	float abs_entries[9];
//	for (int i = 0; i < 3; ++i)
//	{
//		for (int j = 0; j < 3; ++j)
//		{
//			int idx = 3 * j + i;
//			abs_entries[idx] = abs(entries[idx]) + .0f;		// offset to prevent parallel axis but cause representation accuracy issue
//		}
//	}
//	Matrix33 abs_rotation = Matrix33(abs_entries);
//
//	// 4 - test obb1 basis
//	float r1, r2;
//	for (int i = 0; i < 3; ++i)
//	{
//		r1 = obb1.GetHalfExtCopy()[i];
//		r2 = obb2.GetHalfExtCopy()[0] * abs_rotation[i] +
//			obb2.GetHalfExtCopy()[1] * abs_rotation[3 + i] +
//			obb2.GetHalfExtCopy()[2] * abs_rotation[6 + i];
//		if (abs(t[i]) > (r1 + r2))
//			return false;
//	}
//
//	// 5 - test obb2 basis
//	for (int i = 0; i < 3; ++i)
//	{
//		r1 = obb1.GetHalfExtCopy()[0] * abs_rotation[3 * i] +
//			obb1.GetHalfExtCopy()[1] * abs_rotation[3 * i + 1] +
//			obb1.GetHalfExtCopy()[2] * abs_rotation[3 * i + 2];
//		r2 = obb2.GetHalfExtCopy()[i];
//		float dist = abs(t[0] * rotation[3*i] + t[1] * rotation[3*i+1] + t[2] * rotation[3*i+2]); 
//		if (dist > (r1 + r2))
//			return false;
//	}
//
//	// 6 - test axis obb1x cross obb2x
//	r1 = obb1.m_halfExt[1] * abs_rotation[2] + obb1.m_halfExt[2] * abs_rotation[1];
//	r2 = obb2.m_halfExt[1] * abs_rotation[6] + obb2.m_halfExt[2] * abs_rotation[3];
//	float dist = abs(t[2] * rotation[1] - t[1] * rotation[2]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 7 - test axis obb1x cross obb2y
//	r1 = obb1.m_halfExt[1] * abs_rotation[5] + obb1.m_halfExt[2] * abs_rotation[4];
//	r2 = obb2.m_halfExt[0] * abs_rotation[6] + obb2.m_halfExt[2] * abs_rotation[0];
//	dist = abs(t[2] * rotation[4] - t[1] * rotation[5]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 8 - test axis obb1x cross obb2z
//	r1 = obb1.m_halfExt[1] * abs_rotation[8] + obb1.m_halfExt[2] * abs_rotation[7];
//	r2 = obb2.m_halfExt[0] * abs_rotation[3] + obb2.m_halfExt[1] * abs_rotation[0];
//	dist = abs(t[2] * rotation[7] - t[1] * rotation[8]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 9 - test axis obb1y cross obb2x
//	r1 = obb1.m_halfExt[0] * abs_rotation[2] + obb1.m_halfExt[2] * abs_rotation[0];
//	r2 = obb2.m_halfExt[1] * abs_rotation[7] + obb2.m_halfExt[2] * abs_rotation[4];
//	dist = abs(t[0] * rotation[2] - t[2] * rotation[0]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 10 - test axis obb1y cross obb2y
//	r1 = obb1.m_halfExt[0] * abs_rotation[5] + obb1.m_halfExt[2] * abs_rotation[3];
//	r2 = obb2.m_halfExt[0] * abs_rotation[7] + obb2.m_halfExt[2] * abs_rotation[1];
//	dist = abs(t[0] * rotation[5] - t[2] * rotation[3]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 11 - test axis obb1y cross obb2z
//	r1 = obb1.m_halfExt[0] * abs_rotation[8] + obb1.m_halfExt[2] * abs_rotation[6];
//	r2 = obb2.m_halfExt[0] * abs_rotation[4] + obb2.m_halfExt[1] * abs_rotation[1];
//	dist = abs(t[0] * rotation[8] - t[2] * rotation[6]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 12 - test axis obb1z cross obb2x
//	r1 = obb1.m_halfExt[0] * abs_rotation[1] + obb1.m_halfExt[1] * abs_rotation[0];
//	r2 = obb2.m_halfExt[1] * abs_rotation[8] + obb2.m_halfExt[2] * abs_rotation[5];
//	dist = abs(t[1] * rotation[0] - t[0] * rotation[1]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 13 - test axis obb1z cross obb2y
//	r1 = obb1.m_halfExt[0] * abs_rotation[4] + obb1.m_halfExt[1] * abs_rotation[3];
//	r2 = obb2.m_halfExt[0] * abs_rotation[8] + obb2.m_halfExt[2] * abs_rotation[2];
//	dist = abs(t[1] * rotation[3] - t[0] * rotation[4]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// 14 - test axis obb1z cross obb2z
//	r1 = obb1.m_halfExt[0] * abs_rotation[7] + obb1.m_halfExt[1] * abs_rotation[6];
//	r2 = obb2.m_halfExt[0] * abs_rotation[5] + obb2.m_halfExt[1] * abs_rotation[2];
//	dist = abs(t[1] * rotation[6] - t[0] * rotation[7]);
//	if (dist > (r1 + r2))
//		return false;
//
//	// no SAT, mush intersect
//	return true;
//}
//
//static eBoxBoxDebug debug_stat = SECOND_OBB_VERTS;
//static int obb2_vert_idx = 0;
//static int obb1_face_idx = 0;
//static int obb1_vert_idx = 0;
//static int obb2_face_idx = 0;
//static float shallowest_global = -INFINITY;
//static float deepest = INFINITY;
//static int shallowest_pair_obb2_vert_idx;
//static int shallowest_pair_obb1_face_idx;
//static int shallowest_pair_obb1_vert_idx;
//static int shallowest_pair_obb2_face_idx;
//static int deepest_pair_obb2_vert_idx = -1;
//static int deepest_pair_obb1_face_idx = -1;
//static int deepest_pair_obb1_vert_idx = -1;
//static int deepest_pair_obb2_face_idx = -1;
//Mesh* obb2_vert_to_obb1_face_0 = nullptr;
//Mesh* obb2_vert_to_obb1_face_1 = nullptr;
//Mesh* obb2_vert_to_obb1_face_2 = nullptr;
//Mesh* obb2_vert_to_obb1_face_3 = nullptr;
//Mesh* obb2_vert_to_obb1_face_4 = nullptr;
//Mesh* obb2_vert_to_obb1_face_5 = nullptr;
//Mesh* obb1_vert_to_obb2_face_0 = nullptr;
//Mesh* obb1_vert_to_obb2_face_1 = nullptr;
//Mesh* obb1_vert_to_obb2_face_2 = nullptr;
//Mesh* obb1_vert_to_obb2_face_3 = nullptr;
//Mesh* obb1_vert_to_obb2_face_4 = nullptr;
//Mesh* obb1_vert_to_obb2_face_5 = nullptr;
//Mesh* obb2_vert_0_winner = nullptr;
//Mesh* obb2_vert_1_winner = nullptr;
//Mesh* obb2_vert_2_winner = nullptr;
//Mesh* obb2_vert_3_winner = nullptr;
//Mesh* obb2_vert_4_winner = nullptr;
//Mesh* obb2_vert_5_winner = nullptr;
//Mesh* obb2_vert_6_winner = nullptr;
//Mesh* obb2_vert_7_winner = nullptr;
//Mesh* obb1_vert_0_winner = nullptr;
//Mesh* obb1_vert_1_winner = nullptr;
//Mesh* obb1_vert_2_winner = nullptr;
//Mesh* obb1_vert_3_winner = nullptr;
//Mesh* obb1_vert_4_winner = nullptr;
//Mesh* obb1_vert_5_winner = nullptr;
//Mesh* obb1_vert_6_winner = nullptr;
//Mesh* obb1_vert_7_winner = nullptr;
//Mesh* obb2_pt_obb1_face_winner = nullptr;
//Mesh* obb1_pt_obb2_face_winner = nullptr;
//void DeleteOBB2Winner()
//{
//	if (obb2_vert_0_winner != nullptr)
//	{
//		delete obb2_vert_0_winner;
//		obb2_vert_0_winner = nullptr;
//	}
//	if (obb2_vert_1_winner != nullptr)
//	{
//		delete obb2_vert_1_winner;
//		obb2_vert_1_winner = nullptr;
//	}
//	if (obb2_vert_2_winner != nullptr)
//	{
//		delete obb2_vert_2_winner;
//		obb2_vert_2_winner = nullptr;
//	}
//	if (obb2_vert_3_winner != nullptr)
//	{
//		delete obb2_vert_3_winner;
//		obb2_vert_3_winner = nullptr;
//	}
//	if (obb2_vert_4_winner != nullptr)
//	{
//		delete obb2_vert_4_winner;
//		obb2_vert_4_winner = nullptr;
//	}
//	if (obb2_vert_5_winner != nullptr)
//	{
//		delete obb2_vert_5_winner;
//		obb2_vert_5_winner = nullptr;
//	}
//	if (obb2_vert_6_winner != nullptr)
//	{
//		delete obb2_vert_6_winner;
//		obb2_vert_6_winner = nullptr;
//	}
//	if (obb2_vert_7_winner != nullptr)
//	{
//		delete obb2_vert_7_winner;
//		obb2_vert_7_winner = nullptr;
//	}
//}
//
//void DeleteOBB1Winner()
//{
//	if (obb1_vert_0_winner != nullptr)
//	{
//		delete obb1_vert_0_winner;
//		obb1_vert_0_winner = nullptr;
//	}
//	if (obb1_vert_1_winner != nullptr)
//	{
//		delete obb1_vert_1_winner;
//		obb1_vert_1_winner = nullptr;
//	}
//	if (obb1_vert_2_winner != nullptr)
//	{
//		delete obb1_vert_2_winner;
//		obb1_vert_2_winner = nullptr;
//	}
//	if (obb1_vert_3_winner != nullptr)
//	{
//		delete obb1_vert_3_winner;
//		obb1_vert_3_winner = nullptr;
//	}
//	if (obb1_vert_4_winner != nullptr)
//	{
//		delete obb1_vert_4_winner;
//		obb1_vert_4_winner = nullptr;
//	}
//	if (obb1_vert_5_winner != nullptr)
//	{
//		delete obb1_vert_5_winner;
//		obb1_vert_5_winner = nullptr;
//	}
//	if (obb1_vert_6_winner != nullptr)
//	{
//		delete obb1_vert_6_winner;
//		obb1_vert_6_winner = nullptr;
//	}
//	if (obb1_vert_7_winner != nullptr)
//	{
//		delete obb1_vert_7_winner;
//		obb1_vert_7_winner = nullptr;
//	}
//}
//
//void DeleteOBB1PtToOBB2Face()
//{
//	if (obb1_vert_to_obb2_face_0 != nullptr)
//	{
//		delete obb1_vert_to_obb2_face_0;
//		obb1_vert_to_obb2_face_0 = nullptr;
//	}
//	if (obb1_vert_to_obb2_face_1 != nullptr)
//	{
//		delete obb1_vert_to_obb2_face_1;
//		obb1_vert_to_obb2_face_1 = nullptr;
//	}
//	if (obb1_vert_to_obb2_face_2 != nullptr)
//	{
//		delete obb1_vert_to_obb2_face_2;
//		obb1_vert_to_obb2_face_2 = nullptr;
//	}
//	if (obb1_vert_to_obb2_face_3 != nullptr)
//	{
//		delete obb1_vert_to_obb2_face_3;
//		obb1_vert_to_obb2_face_3 = nullptr;
//	}
//	if (obb1_vert_to_obb2_face_4 != nullptr)
//	{
//		delete obb1_vert_to_obb2_face_4;
//		obb1_vert_to_obb2_face_4 = nullptr;
//	}
//	if (obb1_vert_to_obb2_face_5 != nullptr)
//	{
//		delete obb1_vert_to_obb2_face_5;
//		obb1_vert_to_obb2_face_5 = nullptr;
//	}
//}
//
//void DeleteOBB2PtToOBB1Face()
//{
//	if (obb2_vert_to_obb1_face_0 != nullptr)
//	{
//		delete obb2_vert_to_obb1_face_0;
//		obb2_vert_to_obb1_face_0 = nullptr;
//	}
//	if (obb2_vert_to_obb1_face_1 != nullptr)
//	{
//		delete obb2_vert_to_obb1_face_1;
//		obb2_vert_to_obb1_face_1 = nullptr;
//	}
//	if (obb2_vert_to_obb1_face_2 != nullptr)
//	{
//		delete obb2_vert_to_obb1_face_2;
//		obb2_vert_to_obb1_face_2 = nullptr;
//	}
//	if (obb2_vert_to_obb1_face_3 != nullptr)
//	{
//		delete obb2_vert_to_obb1_face_3;
//		obb2_vert_to_obb1_face_3 = nullptr;
//	}
//	if (obb2_vert_to_obb1_face_4 != nullptr)
//	{
//		delete obb2_vert_to_obb1_face_4;
//		obb2_vert_to_obb1_face_4 = nullptr;
//	}
//	if (obb2_vert_to_obb1_face_5 != nullptr)
//	{
//		delete obb2_vert_to_obb1_face_5;
//		obb2_vert_to_obb1_face_5 = nullptr;
//	}
//}
//
//void GenerateObb1PtToObb2FaceMesh(const OBB3Face& obb2_face, float ext)
//{
//	const Rgba& color = color_list[color_index];
//	color_index = (color_index + 1) % COLOR_LIST_SIZE;
//	if (obb2_face_idx == 0)
//	{
//		if (obb1_vert_to_obb2_face_0 != nullptr)
//		{
//			delete obb1_vert_to_obb2_face_0;
//			obb1_vert_to_obb2_face_0 = nullptr;
//		}
//		obb1_vert_to_obb2_face_0 = Mesh::CreateLineImmediate(VERT_PCU, obb2_face.m_center, obb2_face.m_center + obb2_face.m_normal * ext, color);
//	}
//	else if (obb2_face_idx == 1)
//	{
//		if (obb1_vert_to_obb2_face_1 != nullptr)
//		{
//			delete obb1_vert_to_obb2_face_1;
//			obb1_vert_to_obb2_face_1 = nullptr;
//		}
//		obb1_vert_to_obb2_face_1 = Mesh::CreateLineImmediate(VERT_PCU, obb2_face.m_center, obb2_face.m_center + obb2_face.m_normal * ext, color);
//	}
//	else if (obb2_face_idx == 2)
//	{
//		if (obb1_vert_to_obb2_face_2 != nullptr)
//		{
//			delete obb1_vert_to_obb2_face_2;
//			obb1_vert_to_obb2_face_2 = nullptr;
//		}
//		obb1_vert_to_obb2_face_2 = Mesh::CreateLineImmediate(VERT_PCU, obb2_face.m_center, obb2_face.m_center + obb2_face.m_normal * ext, color);
//	}
//	else if (obb2_face_idx == 3)
//	{
//		if (obb1_vert_to_obb2_face_3 != nullptr)
//		{
//			delete obb1_vert_to_obb2_face_3;
//			obb1_vert_to_obb2_face_3 = nullptr;
//		}
//		obb1_vert_to_obb2_face_3 = Mesh::CreateLineImmediate(VERT_PCU, obb2_face.m_center, obb2_face.m_center + obb2_face.m_normal * ext, color);
//	}
//	else if (obb2_face_idx == 4)
//	{
//		if (obb1_vert_to_obb2_face_4 != nullptr)
//		{
//			delete obb1_vert_to_obb2_face_4;
//			obb1_vert_to_obb2_face_4 = nullptr;
//		}
//		obb1_vert_to_obb2_face_4 = Mesh::CreateLineImmediate(VERT_PCU, obb2_face.m_center, obb2_face.m_center + obb2_face.m_normal * ext, color);
//	}
//	else if (obb2_face_idx == 5)
//	{
//		if (obb1_vert_to_obb2_face_5 != nullptr)
//		{
//			delete obb1_vert_to_obb2_face_5;
//			obb1_vert_to_obb2_face_5 = nullptr;
//		}
//		obb1_vert_to_obb2_face_5 = Mesh::CreateLineImmediate(VERT_PCU, obb2_face.m_center, obb2_face.m_center + obb2_face.m_normal * ext, color);
//	}
//}
//
//void GenerateObb2PtToObb1FaceMesh(const OBB3Face& obb1_face, float ext)
//{
//	const Rgba& color = color_list[color_index];
//	color_index = (color_index + 1) % COLOR_LIST_SIZE;
//	if (obb1_face_idx == 0)
//	{
//		if (obb2_vert_to_obb1_face_0 != nullptr)
//		{
//			delete obb2_vert_to_obb1_face_0;
//			obb2_vert_to_obb1_face_0 = nullptr;
//		}
//		obb2_vert_to_obb1_face_0 = Mesh::CreateLineImmediate(VERT_PCU, obb1_face.m_center, obb1_face.m_center + obb1_face.m_normal * ext, color);
//	}
//	else if (obb1_face_idx == 1)
//	{
//		if (obb2_vert_to_obb1_face_1 != nullptr)
//		{
//			delete obb2_vert_to_obb1_face_1;
//			obb2_vert_to_obb1_face_1 = nullptr;
//		}
//		obb2_vert_to_obb1_face_1 = Mesh::CreateLineImmediate(VERT_PCU, obb1_face.m_center, obb1_face.m_center + obb1_face.m_normal * ext, color);
//	}
//	else if (obb1_face_idx == 2)
//	{
//		if (obb2_vert_to_obb1_face_2 != nullptr)
//		{
//			delete obb2_vert_to_obb1_face_2;
//			obb2_vert_to_obb1_face_2 = nullptr;
//		}
//		obb2_vert_to_obb1_face_2 = Mesh::CreateLineImmediate(VERT_PCU, obb1_face.m_center, obb1_face.m_center + obb1_face.m_normal * ext, color);
//	}
//	else if (obb1_face_idx == 3)
//	{
//		if (obb2_vert_to_obb1_face_3 != nullptr)
//		{
//			delete obb2_vert_to_obb1_face_3;
//			obb2_vert_to_obb1_face_3 = nullptr;
//		}
//		obb2_vert_to_obb1_face_3 = Mesh::CreateLineImmediate(VERT_PCU, obb1_face.m_center, obb1_face.m_center + obb1_face.m_normal * ext, color);
//	}
//	else if (obb1_face_idx == 4)
//	{
//		if (obb2_vert_to_obb1_face_4 != nullptr)
//		{
//			delete obb2_vert_to_obb1_face_4;
//			obb2_vert_to_obb1_face_4 = nullptr;
//		}
//		obb2_vert_to_obb1_face_4 = Mesh::CreateLineImmediate(VERT_PCU, obb1_face.m_center, obb1_face.m_center + obb1_face.m_normal * ext, color);
//	}
//	else if (obb1_face_idx == 5)
//	{
//		if (obb2_vert_to_obb1_face_5 != nullptr)
//		{
//			delete obb2_vert_to_obb1_face_5;
//			obb2_vert_to_obb1_face_5 = nullptr;
//		}
//		obb2_vert_to_obb1_face_5 = Mesh::CreateLineImmediate(VERT_PCU, obb1_face.m_center, obb1_face.m_center + obb1_face.m_normal * ext, color);
//	}
//}
//
//void GenerateObb1PtToObb2FaceWinnerMesh(const OBB3Face& shallow_obb2_face, float shallow_ext)
//{
//	if (shallowest_pair_obb1_vert_idx == 0)
//	{
//		if (obb1_vert_0_winner != nullptr)
//		{
//			delete obb1_vert_0_winner;
//			obb1_vert_0_winner = nullptr;
//		}
//		obb1_vert_0_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 1)
//	{
//		if (obb1_vert_1_winner != nullptr)
//		{
//			delete obb1_vert_1_winner;
//			obb1_vert_1_winner = nullptr;
//		}
//		obb1_vert_1_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 2)
//	{
//		if (obb1_vert_2_winner != nullptr)
//		{
//			delete obb1_vert_2_winner;
//			obb1_vert_2_winner = nullptr;
//		}
//		obb1_vert_2_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 3)
//	{
//		if (obb1_vert_3_winner != nullptr)
//		{
//			delete obb1_vert_3_winner;
//			obb1_vert_3_winner = nullptr;
//		}
//		obb1_vert_3_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 4)
//	{
//		if (obb1_vert_4_winner != nullptr)
//		{
//			delete obb1_vert_4_winner;
//			obb1_vert_4_winner = nullptr;
//		}
//		obb1_vert_4_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 5)
//	{
//		if (obb1_vert_5_winner != nullptr)
//		{
//			delete obb1_vert_5_winner;
//			obb1_vert_5_winner = nullptr;
//		}
//		obb1_vert_5_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 6)
//	{
//		if (obb1_vert_6_winner != nullptr)
//		{
//			delete obb1_vert_6_winner;
//			obb1_vert_6_winner = nullptr;
//		}
//		obb1_vert_6_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb1_vert_idx == 7)
//	{
//		if (obb1_vert_7_winner != nullptr)
//		{
//			delete obb1_vert_7_winner;
//			obb1_vert_7_winner = nullptr;
//		}
//		obb1_vert_7_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb2_face.m_center,
//			shallow_obb2_face.m_center + shallow_obb2_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//}
//
//void GenerateObb2PtToObb1FaceWinnerMesh(const OBB3Face& shallow_obb1_face, float shallow_ext)
//{
//	if (shallowest_pair_obb2_vert_idx == 0)
//	{
//		if (obb2_vert_0_winner != nullptr)
//		{
//			delete obb2_vert_0_winner;
//			obb2_vert_0_winner = nullptr;
//		}
//		obb2_vert_0_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 1)
//	{
//		if (obb2_vert_1_winner != nullptr)
//		{
//			delete obb2_vert_1_winner;
//			obb2_vert_1_winner = nullptr;
//		}
//		obb2_vert_1_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 2)
//	{
//		if (obb2_vert_2_winner != nullptr)
//		{
//			delete obb2_vert_2_winner;
//			obb2_vert_2_winner = nullptr;
//		}
//		obb2_vert_2_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 3)
//	{
//		if (obb2_vert_3_winner != nullptr)
//		{
//			delete obb2_vert_3_winner;
//			obb2_vert_3_winner = nullptr;
//		}
//		obb2_vert_3_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 4)
//	{
//		if (obb2_vert_4_winner != nullptr)
//		{
//			delete obb2_vert_4_winner;
//			obb2_vert_4_winner = nullptr;
//		}
//		obb2_vert_4_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 5)
//	{
//		if (obb2_vert_5_winner != nullptr)
//		{
//			delete obb2_vert_5_winner;
//			obb2_vert_5_winner = nullptr;
//		}
//		obb2_vert_5_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 6)
//	{
//		if (obb2_vert_6_winner != nullptr)
//		{
//			delete obb2_vert_6_winner;
//			obb2_vert_6_winner = nullptr;
//		}
//		obb2_vert_6_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//	if (shallowest_pair_obb2_vert_idx == 7)
//	{
//		if (obb2_vert_7_winner != nullptr)
//		{
//			delete obb2_vert_7_winner;
//			obb2_vert_7_winner = nullptr;
//		}
//		obb2_vert_7_winner = Mesh::CreateLineImmediate(VERT_PCU, shallow_obb1_face.m_center,
//			shallow_obb1_face.m_center + shallow_obb1_face.m_normal * shallow_ext, Rgba::GOLD);
//	}
//}
//
//void GenerateOBB1PtToOBB2FaceFinalWinner(const OBB3& obb1, const OBB3& obb2)
//{
//	if (deepest_pair_obb1_vert_idx != -1 && deepest_pair_obb2_face_idx != -1)
//	{
//		const OBB3Vert& deep_obb1_vert = obb1.m_verts[deepest_pair_obb1_vert_idx];
//		const OBB3Face& deep_obb2_face = obb2.m_faces[deepest_pair_obb2_face_idx];
//
//		Vector3 deep_toPt = deep_obb1_vert.m_vert - deep_obb2_face.m_center;
//		float deep_ext = DotProduct(deep_toPt, deep_obb2_face.m_normal);
//
//		if (obb1_pt_obb2_face_winner != nullptr)
//		{
//			delete obb1_pt_obb2_face_winner;
//			obb1_pt_obb2_face_winner = nullptr;
//		}
//		obb1_pt_obb2_face_winner = Mesh::CreateLineImmediate(VERT_PCU, deep_obb2_face.m_center,
//			deep_obb2_face.m_center + deep_obb2_face.m_normal * deep_ext, Rgba::GOLD);
//	}
//}
//
//void GenerateOBB2PtToOBB1FaceFinalWinner(const OBB3& obb1, const OBB3& obb2)
//{
//	if (deepest_pair_obb2_vert_idx != -1 && deepest_pair_obb1_face_idx != -1)
//	{
//		const OBB3Vert& deep_obb2_vert = obb2.m_verts[deepest_pair_obb2_vert_idx];
//		const OBB3Face& deep_obb1_face = obb1.m_faces[deepest_pair_obb1_face_idx];
//
//		Vector3 deep_toPt = deep_obb2_vert.m_vert - deep_obb1_face.m_center;
//		float deep_ext = DotProduct(deep_toPt, deep_obb1_face.m_normal);
//
//		if (obb2_pt_obb1_face_winner != nullptr)
//		{
//			delete obb2_pt_obb1_face_winner;
//			obb2_pt_obb1_face_winner = nullptr;
//		}
//		obb2_pt_obb1_face_winner = Mesh::CreateLineImmediate(VERT_PCU, deep_obb1_face.m_center,
//			deep_obb1_face.m_center + deep_obb1_face.m_normal * deep_ext, Rgba::GOLD);
//	}
//}
//
//void CollisionDetector::OBB3VsOBB3CoreBreakdownPtVsFace(const OBB3& obb1, const OBB3& obb2, Vector3& pt, Vector3& face_center)
//{
//	InputSystem* input = InputSystem::GetInstance();
//	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0) && debug_stat == SECOND_OBB_VERTS)
//	{
//		const OBB3Vert& obb2_vert = obb2.m_verts[obb2_vert_idx];
//		const OBB3Face& obb1_face = obb1.m_faces[obb1_face_idx];
//
//		Vector3 toPt = obb2_vert.m_vert - obb1_face.m_center;
//		float ext = DotProduct(toPt, obb1_face.m_normal);		
//
//		// if this ext is bigger than 0, meaning this vert from obb2 goes beyond certain face, abort
//		// by abort we want to visit next vert of obb2; this current one is meaningless to investigate
//		if (ext > 0.f)
//		{
//			shallowest_global = -INFINITY;
//			obb1_face_idx = 0;
//			obb2_vert_idx++;
//			shallowest_pair_obb2_vert_idx = 0;
//			shallowest_pair_obb1_face_idx = 0;
//
//			// this vert is the last vert of obb2 we check
//			if (obb2_vert_idx == 8)
//			{
//				GenerateOBB2PtToOBB1FaceFinalWinner(obb1, obb2);
//
//				DeleteOBB2Winner();
//
//				deepest = INFINITY;
//				deepest_pair_obb1_face_idx = -1;
//				deepest_pair_obb2_vert_idx = -1;
//				debug_stat = FIRST_OBB_VERTS;
//			}
//
//			DeleteOBB2PtToOBB1Face();
//		}
//		else
//		{
//			// otherwise the point (from obb2) is inside obb1, we need to record the shallowest pen
//			// also, we can render the debug line from face to the point
//			if (ext > shallowest_global)
//			{
//				shallowest_global = ext;
//
//				shallowest_pair_obb2_vert_idx = obb2_vert_idx;
//				shallowest_pair_obb1_face_idx = obb1_face_idx;
//			}
//
//			GenerateObb2PtToObb1FaceMesh(obb1_face, ext);
//
//			obb1_face_idx = (obb1_face_idx + 1) % 6;
//
//			// now we are done with render all candidate for this vert of obb2
//			// we need to select the shallowest one from record
//			if (obb1_face_idx == 0)
//			{
//				// decide on winner of this vert of obb2 against all faces of obb1
//				const OBB3Vert& shallow_obb2_vert = obb2.m_verts[shallowest_pair_obb2_vert_idx];
//				const OBB3Face& shallow_obb1_face = obb1.m_faces[shallowest_pair_obb1_face_idx];
//				Vector3 shallow_toPt = shallow_obb2_vert.m_vert - shallow_obb1_face.m_center;
//				float shallow_ext = DotProduct(shallow_toPt, shallow_obb1_face.m_normal);
//
//				// at the same time we prepare for the final deepest pen check for all vertices
//				if (shallow_ext < deepest)
//				{
//					deepest = shallow_ext;
//
//					deepest_pair_obb2_vert_idx = shallowest_pair_obb2_vert_idx;
//					deepest_pair_obb1_face_idx = shallowest_pair_obb1_face_idx;
//				}
//
//				GenerateObb2PtToObb1FaceWinnerMesh(shallow_obb1_face, shallow_ext);
//
//				shallowest_global = -INFINITY;
//				obb2_vert_idx++;
//				shallowest_pair_obb2_vert_idx = 0;
//				shallowest_pair_obb1_face_idx = 0;
//
//				///////////////////////////////////////////////////////////////////
//				// old debug lines are flushed when picking the winner
//				DeleteOBB2PtToOBB1Face();
//
//				// we are done checking all vertices of obb2, hence done with obb2 itself
//				if (obb2_vert_idx == 8)
//				{
//					GenerateOBB2PtToOBB1FaceFinalWinner(obb1, obb2);
//
//					DeleteOBB2Winner();
//
//					deepest = INFINITY;
//					deepest_pair_obb1_face_idx = -1;
//					deepest_pair_obb2_vert_idx = -1;
//					debug_stat = FIRST_OBB_VERTS;
//				}
//			}
//		}
//	}
//	else if (input->WasKeyJustPressed(InputSystem::KEYBOARD_NUMPAD_0) && debug_stat == FIRST_OBB_VERTS)
//	{
//		const OBB3Vert& obb1_vert = obb1.m_verts[obb1_vert_idx];
//		const OBB3Face& obb2_face = obb2.m_faces[obb2_face_idx];
//
//		Vector3 toPt = obb1_vert.m_vert - obb2_face.m_center;
//		float ext = DotProduct(toPt, obb2_face.m_center);
//
//		if (ext > 0.f)
//		{
//			shallowest_global = -INFINITY;
//			obb2_face_idx = 0;
//			obb1_vert_idx++;
//			shallowest_pair_obb1_vert_idx = 0;
//			shallowest_pair_obb2_face_idx = 0;
//
//			if (obb1_vert_idx == 8)
//			{
//				GenerateOBB1PtToOBB2FaceFinalWinner(obb1, obb2);
//
//				DeleteOBB1Winner();
//
//				deepest = INFINITY;
//				deepest_pair_obb2_face_idx = -1;
//				deepest_pair_obb1_vert_idx = -1;
//				debug_stat = TWO_OBB_EDGES;
//			}
//
//			DeleteOBB1PtToOBB2Face();
//		}
//		else
//		{
//			if (ext > shallowest_global)
//			{
//				shallowest_global = ext;
//
//				shallowest_pair_obb1_vert_idx = obb1_vert_idx;
//				shallowest_pair_obb2_face_idx = obb2_face_idx;
//			}
//
//			GenerateObb1PtToObb2FaceMesh(obb2_face, ext);
//
//			obb2_face_idx = (obb2_face_idx + 1) % 6;
//
//			if (obb2_face_idx == 0)
//			{
//				const OBB3Vert& shallow_obb1_vert = obb1.m_verts[shallowest_pair_obb1_vert_idx];
//				const OBB3Face& shallow_obb2_face = obb2.m_faces[shallowest_pair_obb2_face_idx];
//				Vector3 shallow_toPt = shallow_obb1_vert.m_vert - shallow_obb2_face.m_center;
//				float shallow_ext = DotProduct(shallow_toPt, shallow_obb2_face.m_normal);
//
//				if (shallow_ext < deepest)
//				{
//					deepest = shallow_ext;
//
//					deepest_pair_obb1_vert_idx = shallowest_pair_obb1_vert_idx;
//					deepest_pair_obb2_face_idx = shallowest_pair_obb2_face_idx;
//				}
//
//				GenerateObb1PtToObb2FaceWinnerMesh(shallow_obb2_face, shallow_ext);
//
//				shallowest_global = -INFINITY;
//				obb1_vert_idx++;
//				shallowest_pair_obb1_vert_idx = 0;
//				shallowest_pair_obb2_face_idx = 0;
//
//				DeleteOBB1PtToOBB2Face();
//
//				if (obb1_vert_idx == 8)
//				{
//					GenerateOBB1PtToOBB2FaceFinalWinner(obb1, obb2);
//
//					DeleteOBB1Winner();
//
//					deepest = INFINITY;
//					deepest_pair_obb2_face_idx = -1;
//					deepest_pair_obb1_vert_idx = -1;
//					debug_stat = TWO_OBB_EDGES;
//				}
//			}
//		}
//	}
//
//	switch (debug_stat)
//	{
//	case SECOND_OBB_VERTS:
//	{
//		const OBB3Vert& obb2_vert = obb2.m_verts[obb2_vert_idx];
//		const OBB3Face& obb1_face = obb1.m_faces[obb1_face_idx];
//
//		Vector3 toPt = obb2_vert.m_vert - obb1_face.m_center;
//		float ext = DotProduct(toPt, obb1_face.m_normal);
//
//		DebugRenderLine(0.1f, obb1_face.m_center, 
//			obb1_face.m_center + obb1_face.m_normal * ext, 
//			3.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
//
//		pt = obb2_vert.m_vert;
//		face_center = obb1_face.m_center;
//	}
//		break;
//	case FIRST_OBB_VERTS:
//	{
//		const OBB3Vert& obb1_vert = obb1.m_verts[obb1_vert_idx];
//		const OBB3Face& obb2_face = obb2.m_faces[obb2_face_idx];
//
//		Vector3 toPt = obb1_vert.m_vert - obb2_face.m_center;
//		float ext = DotProduct(toPt, obb2_face.m_normal);
//
//		DebugRenderLine(0.1f, obb2_face.m_center, 
//			obb2_face.m_center + obb2_face.m_normal * ext, 
//			3.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
//
//		pt = obb1_vert.m_vert;
//		face_center = obb2_face.m_center;
//	}
//		break;
//	default:
//		break;
//	}
//}
//
//bool CollisionDetector::OBB3VsOBB3StepOne(const OBB3& obb1, const OBB3& obb2, std::tuple<Vector3, Vector3, float>& c_info2, OBB3Face& obb1_face, OBB3Vert& obb2_vert)
//{
//	//bool res;
//	std::map<Vector3, std::tuple<OBB3Vert, OBB3Face, float>> record2;
//	//std::map<OBB3Vert, std::tuple<OBB3Face, float>> record2;
//	for (std::vector<OBB3Vert>::size_type idx = 0; idx < obb2.m_verts.size(); ++idx)
//	{
//		bool pt_overlap = true;
//		OBB3Vert v = obb2.m_verts[idx];
//		const Vector3& pt = v.m_vert;
//
//		float shallowest = -INFINITY;
//		OBB3Face shallowest_against;
//		std::tuple<OBB3Vert, OBB3Face, float> pt_face_distance;
//
//		for (std::vector<OBB3Face>::size_type idx_face = 0; idx_face < obb1.m_faces.size(); ++idx_face)
//		{
//			const OBB3Face& face = obb1.m_faces[idx_face];
//			Vector3 toPt = pt - face.m_center;
//			const Vector3& face_n = face.m_normal;
//			float ext = DotProduct(toPt, face_n);
//			if (ext > 0.f)
//			{
//				pt_overlap = false;
//				break;
//			}
//
//			if (ext > shallowest)
//			{
//				shallowest = ext;
//				shallowest_against = face;
//			}
//		}
//
//		if (pt_overlap)
//		{
//			std::get<0>(pt_face_distance) = v;
//			std::get<1>(pt_face_distance) = shallowest_against;
//			std::get<2>(pt_face_distance) = abs(shallowest);
//
//			record2.emplace(std::make_pair(pt, pt_face_distance));
//		}
//	}
//
//	float record2_deepest = -INFINITY;
//	//std::tuple<Vector3, Vector3, float> c_info2;
//	for (std::map<Vector3, std::tuple<OBB3Vert, OBB3Face, float>>::iterator it = record2.begin(); it != record2.end(); ++it)
//	//for (std::map<OBB3Vert, std::tuple<OBB3Face, float>>::iterator it = record2.begin(); it != record2.end(); ++it)
//	{
//		const Vector3& pt = it->first;
//		const std::tuple<OBB3Vert, OBB3Face, float>& tup = it->second;
//
//		OBB3Vert v = std::get<0>(tup);
//		OBB3Face face = std::get<1>(tup);
//		float dist = std::get<2>(tup);
//		const Vector3& normal = face.m_normal;
//
//		if (dist > record2_deepest)
//		{
//			record2_deepest = dist;
//
//			std::get<0>(c_info2) = pt;
//			std::get<1>(c_info2) = normal;
//			std::get<2>(c_info2) = dist;
//
//			obb1_face = face;
//			obb2_vert = v;
//		}
//	}
//
//	if (!record2.empty())
//	{
//		//const Vector3& pt = std::get<0>(c_info2);
//		//const Vector3& normal = std::get<1>(c_info2);
//		//const float& dist = std::get<2>(c_info2);
//		//DebugRenderLine(0.1f, pt, pt + normal * dist, 3.f, Rgba::RED, Rgba::RED, DEBUG_RENDER_USE_DEPTH);
//		return true;
//	}
//
//	return false;
//}
//
//bool CollisionDetector::OBB3VsOBB3StepTwo(const OBB3& obb1, const OBB3& obb2, std::tuple<Vector3, Vector3, float>& c_info1, OBB3Face& obb2_face, OBB3Vert& obb1_vert)
//{
//	std::map<Vector3, std::tuple<OBB3Vert, OBB3Face, float>> record1;
//	for (std::vector<OBB3Vert>::size_type idx = 0; idx < obb1.m_verts.size(); ++idx)
//	{
//		bool pt_overlap = true;
//		OBB3Vert v = obb1.m_verts[idx];
//		const Vector3& pt = v.m_vert;
//
//		float shallowest = -INFINITY;
//		OBB3Face shallowest_against;
//		std::tuple<OBB3Vert, OBB3Face, float> pt_face_distance;
//
//		for (std::vector<OBB3Face>::size_type idx_face = 0; idx_face < obb2.m_faces.size(); ++idx_face)
//		{
//			const OBB3Face& face = obb2.m_faces[idx_face];
//			Vector3 toPt = pt - face.m_center;
//			const Vector3& face_n = face.m_normal;
//			float ext = DotProduct(toPt, face_n);
//			if (ext > 0.f)
//			{
//				pt_overlap = false;
//				break;
//			}
//
//			if (ext > shallowest)
//			{
//				shallowest = ext;
//				shallowest_against = face;
//			}
//		}
//
//		if (pt_overlap)
//		{
//			std::get<0>(pt_face_distance) = v;
//			std::get<1>(pt_face_distance) = shallowest_against;
//			std::get<2>(pt_face_distance) = abs(shallowest);
//
//			record1.emplace(std::make_pair(pt, pt_face_distance));
//		}
//	}
//
//	// set the c_info
//	float record1_deepest = -INFINITY;
//	for (std::map<Vector3, std::tuple<OBB3Vert, OBB3Face, float>>::iterator it = record1.begin(); it != record1.end(); ++it)
//	{
//		const Vector3& pt = it->first;
//		const std::tuple<OBB3Vert, OBB3Face, float>& tup = it->second;
//
//		OBB3Vert v = std::get<0>(tup);
//		OBB3Face face = std::get<1>(tup);
//		float dist = std::get<2>(tup);
//		const Vector3& normal = face.m_normal;
//
//		if (dist > record1_deepest)
//		{
//			record1_deepest = dist;
//
//			std::get<0>(c_info1) = pt;
//			std::get<1>(c_info1) = normal;
//			std::get<2>(c_info1) = dist;
//
//			obb2_face = face;
//			obb1_vert = v;
//		}
//	}
//
//	if (!record1.empty())
//	{
//		//const Vector3& pt = std::get<0>(c_info1);
//		//const Vector3& normal = std::get<1>(c_info1);
//		//const float& dist = std::get<2>(c_info1);
//		//DebugRenderLine(0.1f, pt, pt + normal * dist, 3.f, Rgba::GREEN, Rgba::GREEN, DEBUG_RENDER_USE_DEPTH);
//		return true;
//	}
//
//	return false;
//}
//
//bool CollisionDetector::OBB3VsOBB3StepThree(const OBB3& obb1, const OBB3& obb2, std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3>& c_info, OBB3Edge& obb1_edge, OBB3Edge& obb2_edge)
//{
//	// now look at edge to edge contact
//	// map<obb1_edge, tuple<obb2_edge, normal, pen, pt1, pt2>>
//	std::map<OBB3Edge, std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3>> edge_pen_record;
//	for (std::vector<OBB3Edge>::size_type idx_edge1 = 0; idx_edge1 < obb1.m_edges.size(); ++idx_edge1)
//	{
//		bool overlap = false;
//		float shallowest = INFINITY;
//		std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3> edge_edge_penetration;	// <edge, normal, dist, p1, p2>
//
//		const OBB3Edge& edge1 = obb1.m_edges[idx_edge1];
//		LineSegment3 line_seg1 = LineSegment3(edge1.m_end1.m_vert, edge1.m_end2.m_vert);
//
//		for (std::vector<OBB3Edge>::size_type idx_edge2 = 0; idx_edge2 < obb2.m_edges.size(); ++idx_edge2)
//		{
//			const OBB3Edge& edge2 = obb2.m_edges[idx_edge2];
//			LineSegment3 line_seg2 = LineSegment3(edge2.m_end1.m_vert, edge2.m_end2.m_vert);
//
//			Vector3 close_pt1, close_pt2;
//			float closest_sqr = LineSegment3::ClosestPointsSegmentsUnconstrained(line_seg1, line_seg2, close_pt1, close_pt2);
//
//			// if the point is not within the segment, the two edges do not overlap?
//			if (!line_seg1.WithinSegment(close_pt1) || !line_seg2.WithinSegment(close_pt2))
//				continue;
//
//			//DebugRenderLine(0.1f, close_pt1, close_pt2, 3.f, Rgba::MEGENTA, Rgba::MEGENTA, DEBUG_RENDER_USE_DEPTH);
//
//			// trick, if separating, close point on 1 to 2's center should be farther
//			float dist_1_to_2 = (obb2.GetCenter() - close_pt1).GetLengthSquared();
//			float dist_2_to_2 = (obb2.GetCenter() - close_pt2).GetLengthSquared();
//			if (dist_1_to_2 > dist_2_to_2)
//				continue;
//
//			// at the same time, even if dist_12 <= dist_22, the dist between two close pts should not exceeds half of diagonal extension
//			if (closest_sqr > obb2.GetDiagonalHalfSquared())
//				continue;
//
//			float closest_dist = sqrtf(closest_sqr);
//			if (closest_dist < shallowest)
//			{
//				overlap = true;
//
//				shallowest = closest_dist;
//
//				// need to make sure normal is pointing away from 1
//				const Vector3& edge1_vec = line_seg1.extent;
//				const Vector3& edge2_vec = line_seg2.extent;
//				Vector3 cross = edge1_vec.Cross(edge2_vec).GetNormalized();
//				Vector3 ref = edge1.m_end1.m_vert - obb1.GetCenter();
//				if (DotProduct(cross, ref) < 0.f)
//					cross *= -1.f;
//
//				std::get<0>(edge_edge_penetration) = edge2;
//				std::get<1>(edge_edge_penetration) = cross;
//				std::get<2>(edge_edge_penetration) = closest_dist;		// > 0
//				std::get<3>(edge_edge_penetration) = close_pt1;
//				std::get<4>(edge_edge_penetration) = close_pt2;
//			}
//		}
//
//		if (overlap)
//			edge_pen_record.emplace(std::make_pair(edge1, edge_edge_penetration));
//	}
//
//	// debug draw overlap for each edge1 (if there is any)
//	float edge_deepest = -INFINITY;
//	//std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3> res;
//	for (std::map<OBB3Edge, std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3>>::iterator it = edge_pen_record.begin(); it != edge_pen_record.end(); ++it)
//	{
//		OBB3Edge edge1 = it->first;
//		const std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3>& tup = it->second;
//		OBB3Edge edge2 = std::get<0>(tup);
//		//const Vector3& n = std::get<1>(tup);
//		const float& dist = std::get<2>(tup);
//		//const Vector3& pt1 = std::get<3>(tup);
//		//const Vector3& pt2 = std::get<4>(tup);
//
//		//DebugRenderLine(0.1f, pt1, pt2, 5.f, Rgba::MEGENTA, Rgba::MEGENTA, DEBUG_RENDER_USE_DEPTH);
//
//		// pick the deepest dist
//		if (dist > edge_deepest)
//		{
//			edge_deepest = dist;
//			c_info = tup;
//
//			obb1_edge = edge1;
//			obb2_edge = edge2;
//		}
//	}
//
//	if (!edge_pen_record.empty())
//	{
//		//const Vector3& deep_pt1 = std::get<3>(c_info);
//		//const Vector3& deep_pt2 = std::get<4>(c_info);
//		//DebugRenderLine(0.1f, deep_pt1, deep_pt2, 10.f, Rgba::CYAN, Rgba::CYAN, DEBUG_RENDER_USE_DEPTH);
//		return true;
//	}
//
//	return false;
//}
//
//bool CollisionDetector::OBB3VsOBB3Core(const OBB3& obb1, const OBB3& obb2, Contact3& contact)
//{
//	std::tuple<Vector3, Vector3, float> c_info2;
//	OBB3Face obb1_face;
//	OBB3Vert obb2_vert;
//	bool obb2_vert_to_obb1_face = OBB3VsOBB3StepOne(obb1, obb2, c_info2, obb1_face, obb2_vert);
//
//	std::tuple<Vector3, Vector3, float> c_info1;
//	OBB3Vert obb1_vert;
//	OBB3Face obb2_face;
//	bool obb1_vert_to_obb2_face = OBB3VsOBB3StepTwo(obb1, obb2, c_info1, obb2_face, obb1_vert);
//
//	std::tuple<OBB3Edge, Vector3, float, Vector3, Vector3> c_info;
//	OBB3Edge obb1_edge;
//	OBB3Edge obb2_edge;
//	bool obb1_edge_to_obb2_edge = OBB3VsOBB3StepThree(obb1, obb2, c_info, obb1_edge, obb2_edge);
//
//	if (!(obb2_vert_to_obb1_face || obb1_vert_to_obb2_face || obb1_edge_to_obb2_edge))
//		return false;
//
//	// otherwise the two obb3 overlaps	
//	// dist is > 0
//	float dist_21 = -INFINITY;
//	dist_21 = std::get<2>(c_info2);
//
//	float dist_12 = -INFINITY;
//	dist_12 = std::get<2>(c_info1);
//
//	float dist_edge = -INFINITY;
//	dist_edge = std::get<2>(c_info);
//
//	Vector3 usedNormal;
//	Vector3 contactPoint;
//	float penetration;
//	if (max(max(dist_12, dist_21), dist_edge) == dist_21)
//	{
//		const Vector3& normal = std::get<1>(c_info2);
//		const Vector3& vert2 = std::get<0>(c_info2);
//		const float& pen = std::get<2>(c_info2);
//
//		usedNormal = -normal;
//		contactPoint = vert2;
//		penetration = pen;
//
//		// the contact is obb2 vert against obb1 face
//		contact = Contact3(obb1.GetEntity(), obb2.GetEntity(), usedNormal, contactPoint, penetration, .8f, .1f);
//		contact.m_type = POINT_FACE;
//		contact.m_f1 = obb1_face.m_feature;
//		contact.m_f2 = obb2_vert.m_feature;
//	}
//	else if (max(max(dist_12, dist_21), dist_edge) == dist_12)
//	{
//		const Vector3& normal = std::get<1>(c_info1);
//		const Vector3& vert1 = std::get<0>(c_info1);
//		const float& pen = std::get<2>(c_info1);
//
//		usedNormal = normal;
//		contactPoint = vert1 + normal * pen;
//		penetration = pen;
//
//		contact = Contact3(obb1.GetEntity(), obb2.GetEntity(), usedNormal, contactPoint, penetration, .8f, .1f);
//		contact.m_type = POINT_FACE;
//		contact.m_f1 = obb1_vert.m_feature;
//		contact.m_f2 = obb2_face.m_feature;
//	}
//	else
//	{
//		const Vector3& normal = std::get<1>(c_info);
//		const Vector3& close_pt2 = std::get<4>(c_info);
//		const float& pen = std::get<2>(c_info);
//
//		usedNormal = -normal;
//		contactPoint = close_pt2;
//		penetration = pen;
//
//		contact = Contact3(obb1.GetEntity(), obb2.GetEntity(), usedNormal, contactPoint, penetration, .8f, .1f);
//		contact.m_type = EDGE_EDGE;
//		contact.m_f1 = obb1_edge.m_feature;
//		contact.m_f2 = obb2_edge.m_feature;
//	}
//
//	return true;
//}
//
//uint CollisionDetector::OBB3VsOBB3Single(const OBB3& obb1, const OBB3& obb2, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	Contact3 theContact;
//	bool contactGenerated = OBB3VsOBB3Core(obb1, obb2, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//uint CollisionDetector::OBB3VsOBB3Coherent(const OBB3& obb1, const OBB3& obb2, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//		// no contacts amount left, return directly
//		return 0;
//
//	// core of intersection test: sphere vs plane
//	Contact3 theContact;
//	bool contactGenerated = OBB3VsOBB3Core(obb1, obb2, theContact);
//
//	if (!contactGenerated)
//		return 0;
//
//	bool existed = data->HasAndUpdateContact(theContact);
//
//	if (!existed)
//		data->m_contacts.push_back(theContact);
//
//	return 1;
//}
//
//
//uint CollisionDetector::OBB3VsPoint(const OBB3& obb, const Vector3& p, Contact3& contact, bool reverse)
//{
//	BoxEntity3* ent = static_cast<BoxEntity3*>(obb.GetEntity());
//
//	Vector3 local = Transform::WorldToLocalOrthogonal(p, ent->GetEntityTransform());
//
//	Vector3 normal;
//	float shallow = obb.GetHalfExt().x - abs(local.x);
//	if (shallow < 0.f) return 0;
//	normal = obb.m_right.GetNormalized() * ((local.x < 0.f) ? -1.f : 1.f);		// in world coord
//	if (reverse)
//		normal *= -1.f;
//
//	float depth = obb.GetHalfExt().y - abs(local.y);
//	if (depth < 0.f) return 0;
//	else if (depth < shallow)
//	{
//		shallow = depth;
//		normal = obb.m_up.GetNormalized() * ((local.y < 0.f) ? -1.f : 1.f);	// in world
//		if (reverse)
//			normal *= -1.f;
//	}
//
//	depth = obb.GetHalfExt().z - abs(local.z);
//	if (depth < 0.f) return 0;
//	else if  (depth < shallow)
//	{
//		shallow = depth;
//		normal = obb.m_forward.GetNormalized() * ((local.z < 0.f) ? -1.f : 1.f);	// in world
//		if (reverse)
//			normal *= -1.f;
//	}
//
//	// assumption: contact entity already set at this point
//	contact.m_normal = normal;
//	contact.m_penetration = shallow;
//	contact.m_point = p;
//	
//	return 1;
//}
//
////uint CollisionDetector::Entity3VsEntity3(Entity3* e1, Entity3* e2, CollisionData3* data)
////{
////	uint res = 0;
////
////	// 1
////	SphereEntity3* s1 = dynamic_cast<SphereEntity3*>(e1);
////	SphereRB3* srb1 = dynamic_cast<SphereRB3*>(e1);
////
////	CubeEntity3* c1 = dynamic_cast<CubeEntity3*>(e1);
////
////	QuadEntity3* q1 = dynamic_cast<QuadEntity3*>(e1);
////
////	// 2
////	SphereEntity3* s2 = dynamic_cast<SphereEntity3*>(e2);
////	SphereRB3* srb2 = dynamic_cast<SphereRB3*>(e2);
////
////	CubeEntity3* c2 = dynamic_cast<CubeEntity3*>(e2);
////
////	QuadEntity3* q2 = dynamic_cast<QuadEntity3*>(e2);
////
////	if (s1 != nullptr)
////	{
////		if (s2 != nullptr)
////		{
////			Sphere3 sph1 = s1->GetSpherePrimitive();
////			Sphere3 sph2 = s2->GetSpherePrimitive();
////
////			res = Sphere3VsSphere3Single(sph1, sph2, data);
////		}
////		else if (srb2 != nullptr)
////		{
////			Sphere3 sph1 = s1->GetSpherePrimitive();
////			Sphere3 sph2 = srb2->GetSpherePrimitive();
////
////			res = Sphere3VsSphere3Single(sph1, sph2, data);
////		}
////		else if (c2 != nullptr)
////		{
////			Sphere3 sph = s1->GetSpherePrimitive();
////			AABB3 aabb3 = c2->GetCubePrimitive();
////
////			res = Sphere3VsAABB3(sph, aabb3, data);
////		}
////		else if (q2 != nullptr)
////		{
////			Sphere3 sph = s1->GetSpherePrimitive();
////			Plane pl = q2->GetPlanePrimitive();
////
////			res = Sphere3VsPlane3Single(sph, pl, data);
////		}
////	}
////	else if (srb1 != nullptr)
////	{
////		if (s2 != nullptr)
////		{
////			Sphere3 sph1 = srb1->GetSpherePrimitive();
////			Sphere3 sph2 = s2->GetSpherePrimitive();
////
////			res = Sphere3VsSphere3Single(sph1, sph2, data);
////		}
////		else if (srb2 != nullptr)
////		{
////			Sphere3 sph1 = srb1->GetSpherePrimitive();
////			Sphere3 sph2 = srb2->GetSpherePrimitive();
////
////			res = Sphere3VsSphere3Single(sph1, sph2, data);
////		}
////		else if (c2 != nullptr)
////		{
////			Sphere3 sph = srb1->GetSpherePrimitive();
////			AABB3 aabb3 = c2->GetCubePrimitive();
////
////			res = Sphere3VsAABB3(sph, aabb3, data);
////		}
////		else if (q2 != nullptr)
////		{
////			Sphere3 sph = srb1->GetSpherePrimitive();
////			Plane pl = q2->GetPlanePrimitive();
////
////			res = Sphere3VsPlane3Single(sph, pl, data);
////		}
////	}
////	else if (c1 != nullptr)
////	{
////		if (s2 != nullptr)
////		{
////			AABB3 aabb3 = c1->GetCubePrimitive();
////			Sphere3 sph = s2->GetSpherePrimitive();
////
////			res = Sphere3VsAABB3(sph, aabb3, data);
////		}
////		else if (srb2 != nullptr)
////		{
////			AABB3 aabb3 = c1->GetCubePrimitive();
////			Sphere3 sph = srb2->GetSpherePrimitive();
////
////			res = Sphere3VsAABB3(sph, aabb3, data);
////		}
////		else if (c2 != nullptr)
////		{
////			AABB3 aabb3_1 = c1->GetCubePrimitive();
////			AABB3 aabb3_2 = c2->GetCubePrimitive();
////
////			res = AABB3VsAABB3Single(aabb3_1, aabb3_2, data);
////		}
////		else if (q2 != nullptr)
////		{
////			AABB3 aabb3 = c1->GetCubePrimitive();
////			Plane pl = q2->GetPlanePrimitive();
////
////			res = AABB3VsPlane3Single(aabb3, pl, data);
////		}
////	}
////	else if (q1 != nullptr)
////	{
////		if (s2 != nullptr)
////		{
////			Plane pl = q1->GetPlanePrimitive();
////			Sphere3 sph = s2->GetSpherePrimitive();
////
////			res = Sphere3VsPlane3Single(sph, pl, data);
////		}
////		else if (srb2 != nullptr)
////		{
////			Plane pl = q1->GetPlanePrimitive();
////			Sphere3 sph = srb2->GetSpherePrimitive();
////
////			res = Sphere3VsPlane3Single(sph, pl, data);
////		}
////		else if (c2 != nullptr)
////		{
////			Plane pl = q1->GetPlanePrimitive();
////			AABB3 aabb = c2->GetCubePrimitive();
////
////			res = AABB3VsPlane3Single(aabb, pl, data);
////		}
////		else if (q2 != nullptr)
////		{
////			TODO("Later deal with quad vs quad");
////		}
////	}
////
////	return res;
////}
//
//uint CollisionDetector::Rigid3VsRigid3(Rigidbody3* rb1, Rigidbody3* rb2, CollisionData3* data)
//{
//	uint res;
//
//	const eBodyShape& shape1 = rb1->m_body_shape;
//	const eBodyShape& shape2 = rb2->m_body_shape;
//
//	if (shape1 == SHAPE_SPHERE && shape2 == SHAPE_SPHERE)
//	{
//		SphereRB3* srb1 = static_cast<SphereRB3*>(rb1);
//		SphereRB3* srb2 = static_cast<SphereRB3*>(rb2);
//
//		const Sphere3& sph1 = srb1->GetSpherePrimitive();
//		const Sphere3& sph2 = srb2->GetSpherePrimitive();
//
//		res = CollisionDetector::Sphere3VsSphere3Coherent(sph1, sph2, data);
//	}
//	else if (shape1 == SHAPE_SPHERE && shape2 == SHAPE_PLANE)
//	{
//		SphereRB3* srb = static_cast<SphereRB3*>(rb1);
//		QuadRB3* qrb = static_cast<QuadRB3*>(rb2);
//
//		const Sphere3& sph = srb->GetSpherePrimitive();
//		const Plane& pl = qrb->GetPlanePrimitive();
//
//		res = CollisionDetector::Sphere3VsPlane3Coherent(sph, pl, data);
//	}
//	else if (shape1 == SHAPE_SPHERE && shape2 == SHAPE_BOX)
//	{
//		SphereRB3* srb = static_cast<SphereRB3*>(rb1);
//		BoxRB3* brb = static_cast<BoxRB3*>(rb2);
//
//		const Sphere3& sph = srb->GetSpherePrimitive();
//		const OBB3& obb = brb->GetBoxPrimitive();
//
//		res = CollisionDetector::OBB3VsSphere3Coherent(obb, sph, data);
//	}
//	else if (shape1 == SHAPE_PLANE && shape2 == SHAPE_SPHERE)
//	{
//		QuadRB3* qrb = static_cast<QuadRB3*>(rb1);
//		SphereRB3* srb = static_cast<SphereRB3*>(rb2);
//
//		const Plane& pl = qrb->GetPlanePrimitive();
//		const Sphere3& sph = srb->GetSpherePrimitive();
//
//		res = CollisionDetector::Sphere3VsPlane3Coherent(sph, pl, data);
//	}
//	else if (shape1 == SHAPE_PLANE && shape2 == SHAPE_BOX)
//	{
//		QuadRB3* qrb = static_cast<QuadRB3*>(rb1);
//		BoxRB3* brb = static_cast<BoxRB3*>(rb2);
//
//		const Plane& pl = qrb->GetPlanePrimitive();
//		const OBB3& obb = brb->GetBoxPrimitive();
//
//		res = CollisionDetector::OBB3VsPlane3Coherent(obb, pl, data);
//	}
//	else if (shape1 == SHAPE_BOX && shape2 == SHAPE_BOX)
//	{
//		BoxRB3* brb1 = static_cast<BoxRB3*>(rb1);
//		BoxRB3* brb2 = static_cast<BoxRB3*>(rb2);
//
//		const OBB3& obb1 = brb1->GetBoxPrimitive();
//		const OBB3& obb2 = brb2->GetBoxPrimitive();
//
//		res = CollisionDetector::OBB3VsOBB3Coherent(obb1, obb2, data);
//	}
//	else if (shape1 == SHAPE_BOX && shape2 == SHAPE_PLANE)
//	{
//		BoxRB3* brb = static_cast<BoxRB3*>(rb1);
//		QuadRB3* qrb = static_cast<QuadRB3*>(rb2);
//
//		const OBB3& obb = brb->GetBoxPrimitive();
//		const Plane& pl = qrb->GetPlanePrimitive();
//
//		res = CollisionDetector::OBB3VsPlane3Coherent(obb, pl, data);
//	}
//	else if (shape1 == SHAPE_BOX && shape2 == SHAPE_SPHERE)
//	{
//		BoxRB3* brb = static_cast<BoxRB3*>(rb1);
//		SphereRB3* srb = static_cast<SphereRB3*>(rb2);
//
//		const OBB3& obb = brb->GetBoxPrimitive();
//		const Sphere3& sph = srb->GetSpherePrimitive();
//
//		res = CollisionDetector::OBB3VsSphere3Coherent(obb, sph, data);
//	}
//
//	return res;
//}
//
//uint CollisionDetector::AABB3VsPlane3Coherent(const AABB3& aabb, const Plane& plane, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//	{
//		// no contacts amount left, return directly
//		return 0;
//	}
//
//	bool intersected = AABB3VsPlaneIntersection(aabb, plane);
//
//	if (!intersected)
//	{
//		return 0;
//	}
//	else
//	{
//		// there is/are some vertex/vertices of aabb intersecting the plane
//		Vector3 center = aabb.GetCenter();
//		Vector3 vertices[8];
//		aabb.GetVertices(vertices);
//
//		// we need to know which one/ones is/are colliding exactly
//		for (uint i = 0; i < 8; ++i)
//		{
//			Vector3 vert = vertices[i];
//
//			float vertDot = DotProduct(vert, plane.GetNormal().GetNormalized());
//			float signedVertDist = vertDot - plane.GetOffset();
//
//			float centerDot = DotProduct(center, plane.GetNormal().GetNormalized());
//			float signedCenterDist = centerDot - plane.GetOffset();
//
//			// check aabb is colliding from which side of plane
//			// if aabb is colliding from front side of plane
//			if (signedCenterDist > 0)
//			{
//				if (signedVertDist < 0 && data->m_contacts.size() < data->m_maxContacts)
//				{
//					// collided
//					Vector3 usedNormal;
//					float penetration;
//					Vector3 contactPt;			// contact point is on the plane
//
//					usedNormal = plane.GetNormal().GetNormalized();
//					penetration = -signedVertDist;
//					contactPt = vert + usedNormal * penetration;
//
//					Contact3 theContact = Contact3(aabb.GetEntity(), plane.GetEntity(),
//						usedNormal, contactPt, penetration);
//					data->m_contacts.push_back(theContact);
//				}
//				else
//				{
//					// this vertex not colliding, check the next one
//					continue;
//				}
//			}
//
//			// if aabb is colliding from back side of plane
//			else
//			{
//				if (signedVertDist > 0 && data->m_contacts.size() < data->m_maxContacts)
//				{
//					// collided
//					Vector3 usedNormal;
//					float penetration;
//					Vector3 contactPt;			// contact point is on the plane
//
//					usedNormal = -plane.GetNormal().GetNormalized();
//					penetration = signedVertDist;
//					contactPt = vert + usedNormal * penetration;
//
//					Contact3 theContact = Contact3(aabb.GetEntity(), plane.GetEntity(),
//						usedNormal, contactPt, penetration);
//					data->m_contacts.push_back(theContact);
//				}
//				else
//				{
//					continue;
//				}
//			}
//		}
//
//		return 1;
//	}
//}
//
//
//uint CollisionDetector::AABB3VsPlane3Single(const AABB3& aabb, const Plane& plane, CollisionData3* data)
//{
//	if (data->m_contacts.size() >= data->m_maxContacts)
//	{
//		// no contacts amount left, return directly
//		return 0;
//	}
//
//	bool intersected = AABB3VsPlaneIntersection(aabb, plane);
//
//	if (!intersected)
//	{
//		return 0;
//	}
//	else
//	{
//		// keep the deepest penetration
//		Contact3 used_contact = Contact3();
//		used_contact.m_e1 = aabb.GetEntity();
//		used_contact.m_e2 = plane.GetEntity();
//		float max_penetration = -INFINITY;
//		// there is/are some vertex/vertices of aabb intersecting the plane
//		Vector3 center = aabb.GetCenter();
//		Vector3 vertices[8];
//		aabb.GetVertices(vertices);
//
//		// we need to know which one/ones is/are colliding exactly
//		for (uint i = 0; i < 8; ++i)
//		{
//			Vector3 vert = vertices[i];
//
//			float vertDot = DotProduct(vert, plane.GetNormal().GetNormalized());
//			float signedVertDist = vertDot - plane.GetOffset();
//
//			float centerDot = DotProduct(center, plane.GetNormal().GetNormalized());
//			float signedCenterDist = centerDot - plane.GetOffset();
//
//			// check aabb is colliding from which side of plane
//			// if aabb is colliding from front side of plane
//			if (signedCenterDist > 0)
//			{
//				if (signedVertDist < 0 && data->m_contacts.size() < data->m_maxContacts)
//				{
//					if (abs(signedVertDist) > max_penetration)
//					{
//						// collided
//						Vector3 usedNormal;
//						float penetration;
//						Vector3 contactPt;			// contact point is on the plane
//
//						usedNormal = plane.GetNormal().GetNormalized();
//						penetration = -signedVertDist;
//						contactPt = vert + usedNormal * penetration;
//
//						used_contact.m_normal = usedNormal;
//						used_contact.m_point = contactPt;
//						used_contact.m_penetration = penetration;
//
//						//data->m_contacts.push_back(used_contact);
//					}
//				}
//				else
//				{
//					// this vertex not colliding, check the next one
//					continue;
//				}
//			}
//
//			// if aabb is colliding from back side of plane
//			else
//			{
//				if (signedVertDist > 0 && data->m_contacts.size() < data->m_maxContacts)
//				{
//					if (signedVertDist > max_penetration)
//					{
//						// collided
//						Vector3 usedNormal;
//						float penetration;
//						Vector3 contactPt;			// contact point is on the plane
//
//						usedNormal = -plane.GetNormal().GetNormalized();
//						penetration = signedVertDist;
//						contactPt = vert + usedNormal * penetration;
//
//						used_contact.m_normal = usedNormal;
//						used_contact.m_point = contactPt;
//						used_contact.m_penetration = penetration;
//
//						//data->m_contacts.push_back(used_contact);
//					}
//				}
//				else
//				{
//					continue;
//				}
//			}
//		}
//
//		data->m_contacts.push_back(used_contact);
//		return 1;
//	}
//}
