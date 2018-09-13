#include "Engine/Math/Trajectory.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <math.h>


Trajectory::Trajectory()
{

}


Trajectory::Trajectory(float g) : m_g(g)
{

}


Trajectory::~Trajectory()
{

}


Vector2 Trajectory::Evaluate(float gravity, Vector2 launchVelocity, float time)
{
	float spd = launchVelocity.GetLength();
	float angle = launchVelocity.GetOrientationDegrees0To360();

	Vector2 res = Evaluate(gravity, spd, angle, time);
	return res;
}


Vector2 Trajectory::Evaluate(float gravity, float launchSpd, float launchAngle, float time)
{
	Vector2 res;

	float x = launchSpd * CosDegrees(launchAngle) * time;
	float y = (-1.f / 2.f) * gravity * time * time + launchSpd * SinDegrees(launchAngle) * time;

	res.x = x;
	res.y = y;

	return res;
}


float Trajectory::GetMinimumLaunchSpd(float gravity, float distance)
{
	// assume launch angle of 45 deg
	m_launchAngle = 45.f;

	// given angle, get spd
	m_launchSpd = sqrtf(gravity * distance);

	return m_launchSpd;
}


float Trajectory::GetMinimumLaunchSpdWithVertDisplacement(float gravity, float distance, float vertDisplacement)
{
	// assume launch angle of 45 deg
	m_launchAngle = 45.f;
	
	// given angle, get spd
	if (distance > vertDisplacement)
	{
		m_launchSpd = sqrtf((gravity * distance * distance) / (distance - vertDisplacement));
	}
	else
	{
		m_launchSpd = 0.f;
	}

	return m_launchSpd;

}


//-------------------------------------------------------------------------------
// number of points >=3
void Trajectory::EvaluatePoints(float distance, int numPoints)
{
	m_trajecLocalPos.clear();
	float elapsed = 0.f;

	int pointsInBetween = numPoints - 2;
	int intervalNum = pointsInBetween + 1;

	float totalTime = distance / (m_launchSpd * CosDegrees(m_launchAngle));
	float intervalTime = totalTime / intervalNum;
	
	Vector2 launchVel = Vector2::MakeDirectionAtDegrees(m_launchAngle) * m_launchSpd;

	for (int tCount = 0; tCount < numPoints; ++tCount)
	{
		Vector2 point = Trajectory::Evaluate(m_g, launchVel, elapsed);
		m_trajecLocalPos.push_back(point);

		if (tCount != (numPoints - 1))
		{
			elapsed += intervalTime;	
		}
	}
}


void Trajectory::Render(Renderer*)
{
	Vector3 lastPos;
	Vector3 thisPos;

	if (m_trajecWorldPos.size() != 0)
	{
		for (std::vector<Vector3>::size_type worldPosCount = 0; worldPosCount < m_trajecWorldPos.size(); ++worldPosCount)
		{
			if (worldPosCount == 0)
			{
				lastPos = m_trajecWorldPos[worldPosCount];
				thisPos = m_trajecWorldPos[worldPosCount];
			}
			else 
			{
				lastPos = m_trajecWorldPos[worldPosCount - 1];
				thisPos = m_trajecWorldPos[worldPosCount];
			}

			if (lastPos != thisPos)
			{
				//renderer->DrawLine3D(lastPos, thisPos, Rgba::WHITE, Rgba::WHITE, 3.f);
			}
		}
	}
}