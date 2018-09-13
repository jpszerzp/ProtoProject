#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <vector>

class Trajectory
{
private:
	//float m_time;
	float m_g;
	float m_launchSpd;
	float m_launchAngle;

	std::vector<Vector2> m_trajecLocalPos;
	std::vector<Vector3> m_trajecWorldPos;

public:
	Trajectory();
	Trajectory(float g);
	~Trajectory();

	void Render(Renderer* renderer);

	void SetTrajecWorldPos(std::vector<Vector3>& pos) { m_trajecWorldPos = pos; }
	void SetLaunchSpd(float spd) { m_launchSpd = spd; }
	void SetLaunchDeg(float angle) { m_launchAngle = angle; }
	void SetGravityConst(float g) { m_g = g; }

	void EvaluatePoints(float distance, int numPoints);
	static Vector2 Evaluate(float gravity, Vector2 launchVelocity, float time);
	static Vector2 Evaluate(float gravity, float launchSpd, float launchAngle, float time);

	float GetMinimumLaunchSpd(float gravity, float distance);
	float GetMinimumLaunchSpdWithVertDisplacement(float gravity, float distance, float vertDisplacement);
	float GetGravityConst() const { return m_g; }
	float GetLaunchAngle() const { return m_launchAngle; }
	float GetLaunchSpd() const { return m_launchSpd; }
	std::vector<Vector2>& GetTrajecLocalPos() { return m_trajecLocalPos; }
	std::vector<Vector3>& GetTrajecWorldPos() { return m_trajecWorldPos; }
};