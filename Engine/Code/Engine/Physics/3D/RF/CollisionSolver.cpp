#include "Engine/Physics/3D/RF/CollisionSolver.hpp"
#include "Engine/Math/MathUtils.hpp"

CollisionSolver::CollisionSolver(uint itr, float v_threshold, float p_threshold)
{
	SetIterations(itr);
	SetThresholds(v_threshold, p_threshold);
}

CollisionSolver::CollisionSolver(uint v_itr, uint p_itr, float v_threshold, float p_threshold)
{
	SetIterations(v_itr, p_itr);
	SetThresholds(v_threshold, p_threshold);
}

bool CollisionSolver::IsValid()
{
	return (m_vel_iterations > 0) && (m_pos_iterations > 0) && (m_vel_threshold >= 0.f) && (m_pos_threshold >= 0.f);
}

void CollisionSolver::SolveCollision(Collision* collisions, uint collision_num, float duration)
{
	// if no collision to solve, return
	if (collision_num == 0)
		return;

	if (!IsValid())
	{
		ASSERT_RECOVERABLE(false, "solver settings are incorrect");
		return;
	}

	PrepareCollision(collisions, collision_num, duration);

	SolvePositions(collisions, collision_num, duration);

	SolveVelocities(collisions, collision_num, duration);
}

void CollisionSolver::SetIterations(uint v_itr, uint p_itr)
{
	m_vel_iterations = v_itr;
	m_pos_iterations = p_itr;
}

void CollisionSolver::SetIterations(uint itr)
{
	SetIterations(itr, itr);
}

void CollisionSolver::SetThresholds(const float& v_thres, const float& p_thres)
{
	m_vel_threshold = v_thres;
	m_pos_threshold = p_thres;
}

void CollisionSolver::PrepareCollision(Collision* collisions, uint collision_num, float duration)
{
	Collision* last_collision = collisions + collision_num;

	for (Collision* col = collisions; col != last_collision; ++col)
	{
		col->CacheData(duration);
	}
}

void CollisionSolver::SolveVelocities(Collision* collisions, uint collision_num, float duration)
{
	Vector3 velocityChange[2], rotationChange[2];
	Vector3 deltaVel;

	// iteratively handle impacts in order of severity.
	m_v_itr_used = 0;
	while (m_v_itr_used < m_vel_iterations)
	{
		// Find contact with maximum magnitude of probable velocity change.
		float max = m_vel_threshold;
		unsigned index = collision_num;
		for (unsigned i = 0; i < collision_num; i++)
		{
			if (collisions[i].m_desired_vel > max)
			{
				max = collisions[i].m_desired_vel;
				index = i;
			}
		}
		if (index == collision_num)
			break;

		// Match the awake state at the contact
		collisions[index].CheckAwake();

		// Do the resolution on the contact that came out top.
		collisions[index].ApplyVelocityChange(velocityChange, rotationChange);

		// With the change in velocity of the two bodies, the update of
		// contact velocities means that some of the relative closing
		// velocities need recomputing.
		for (unsigned i = 0; i < collision_num; i++)
		{
			// Check each body in the contact
			for (unsigned b = 0; b < 2; b++) 
			{
				if (collisions[i].m_bodies[b])
				{
					// Check for a match with each body in the newly
					// resolved contact
					for (unsigned d = 0; d < 2; d++)
					{
						if (collisions[i].m_bodies[b] == collisions[index].m_bodies[d])
						{
							deltaVel = velocityChange[d] + rotationChange[d].Cross(
									collisions[i].m_relative_pos[b]);

							// The sign of the change is negative if we're dealing
							// with the second body in a contact.
							collisions[i].m_closing_vel += collisions[i].m_to_world.MultiplyTranspose(deltaVel) * (b?-1.f:1.f);
							collisions[i].ComputeDesiredDeltaVelocity(duration);
						}
					}
				}
			}
		}
		m_v_itr_used++;
	}
}

void CollisionSolver::SolvePositions(Collision* collisions, uint collision_num, float)
{
	unsigned i, index;
	Vector3 linearChange[2], angularChange[2];
	float max;
	Vector3 deltaPosition;

	// iteratively resolve interpenetrations in order of severity.
	m_p_itr_used = 0;
	while (m_p_itr_used < m_pos_iterations)
	{
		// Find biggest penetration
		max = m_pos_threshold;
		index = collision_num;
		for (i=0; i<collision_num; i++)
		{
			if (collisions[i].GetPenetration() > max)
			{
				max = collisions[i].GetPenetration();
				index = i;
			}
		}
		if (index == collision_num) 
			break;

		// Match the awake state at the contact
		collisions[index].CheckAwake();

		// Resolve the penetration.
		collisions[index].ApplyPositionChange(linearChange, angularChange, max);

		// Again this action may have changed the penetration of other
		// bodies, so we update contacts.
		for (i = 0; i < collision_num; i++)
		{
			// Check each body in the contact
			for (unsigned b = 0; b < 2; b++) 
			{
				if (collisions[i].m_bodies[b])
				{
					// Check for a match with each body in the newly
					// resolved contact
					for (unsigned d = 0; d < 2; d++)
					{
						if (collisions[i].m_bodies[b] == collisions[index].m_bodies[d])
						{
							deltaPosition = linearChange[d] + angularChange[d].Cross(
								collisions[i].m_relative_pos[b]);

							// The sign of the change is positive if we're
							// dealing with the second body in a contact
							// and negative otherwise (because we're
							// subtracting the resolution)..
							collisions[i].m_penetration += DotProduct(
								deltaPosition, collisions[i].m_normal) * (b ? 1 : -1);
						}
					}
				}
			}
		}
		m_p_itr_used++;
	}
}

