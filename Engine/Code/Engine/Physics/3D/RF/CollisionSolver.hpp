#pragma once

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/3D/RF/TheCollision.hpp"

class CollisionSolver
{
	uint m_vel_iterations;
	uint m_pos_iterations;

	float m_vel_threshold;
	float m_pos_threshold;

	uint m_v_itr_used;
	uint m_p_itr_used;

public:
	CollisionSolver(){}
	CollisionSolver(uint itr, float v_threshold, float p_threshold);
	CollisionSolver(uint v_itr, uint p_itr, float v_threshold, float p_threshold);

	bool IsValid();

	void SolveCollision(Collision* collisions, uint collision_num, float duration);

	void SetIterations(uint v_itr, uint p_itr)
	{
		m_vel_iterations = v_itr;
		m_pos_iterations = p_itr;
	}

	void SetIterations(uint itr)
	{
		SetIterations(itr, itr);
	}

	void SetThresholds(const float& v_thres, const float& p_thres)
	{
		m_vel_threshold = v_thres;
		m_pos_threshold = p_thres;
	}

protected:
	void PrepareCollision(Collision* collisions, uint collision_num, float duration);
	void SolveVelocities(Collision* collisions, uint collision_num, float duration);
	void SolvePositions(Collision* collisions, uint collision_num, float duration);
};