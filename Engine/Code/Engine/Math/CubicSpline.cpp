#include "Engine/Math/CubicSpline.hpp"
#include "Engine/Math/MathUtils.hpp"


CubicSpline2D::CubicSpline2D(const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray)
{
	if (velocitiesArray != nullptr)
	{
		for (int dataCount = 0; dataCount < numPoints; ++dataCount)
		{
			Vector2 pos = positionsArray[dataCount];
			Vector2 vel = velocitiesArray[dataCount];
			m_positions.push_back(pos);
			m_velocities.push_back(vel);
		}
	}
	else
	{
		for (int dataCount = 0; dataCount < numPoints; ++dataCount)
		{
			Vector2 pos = positionsArray[dataCount];
			Vector2 vel = Vector2(0.f, 0.f);
			m_positions.push_back(pos);
			m_velocities.push_back(vel);
		}
	}
}


void	CubicSpline2D::AppendPoint( const Vector2& position, const Vector2& velocity )
{
	m_positions.push_back(position);
	m_velocities.push_back(velocity);
}


void	CubicSpline2D::AppendPoints( const Vector2* positionsArray, int numPoints, const Vector2* velocitiesArray )
{
	if (velocitiesArray != nullptr)
	{
		for (int dataCount = 0; dataCount < numPoints; ++dataCount)
		{
			Vector2 pos = positionsArray[dataCount];
			Vector2 vel = velocitiesArray[dataCount];
			m_positions.push_back(pos);
			m_velocities.push_back(vel);
		}
	}
	else
	{
		for (int dataCount = 0; dataCount < numPoints; ++dataCount)
		{
			Vector2 pos = positionsArray[dataCount];
			Vector2 vel = Vector2(0.f, 0.f);
			m_positions.push_back(pos);
			m_velocities.push_back(vel);
		}
	}
}


void	CubicSpline2D::InsertPoint( int insertBeforeIndex, const Vector2& position, const Vector2& velocity )
{
	std::vector<Vector2>::iterator posIt = m_positions.begin() + insertBeforeIndex;
	m_positions.insert(posIt, position);

	std::vector<Vector2>::iterator velIt = m_velocities.begin() + insertBeforeIndex;
	m_velocities.insert(velIt, velocity);
}


void	CubicSpline2D::RemovePoint( int pointIndex )
{
	std::vector<Vector2>::iterator posIt = m_positions.begin() + pointIndex;
	m_positions.erase(posIt);

	std::vector<Vector2>::iterator velIt = m_velocities.begin() + pointIndex;
	m_velocities.erase(velIt);
}


void	CubicSpline2D::RemoveAllPoints()
{
	m_positions.clear();
	m_velocities.clear();
}


void	CubicSpline2D::SetPoint( int pointIndex, const Vector2& newPosition, const Vector2& newVelocity )
{
	m_positions[pointIndex] = newPosition;
	m_velocities[pointIndex] = newVelocity;
}


void	CubicSpline2D::SetPosition( int pointIndex, const Vector2& newPosition )
{
	m_positions[pointIndex] = newPosition;
}


void	CubicSpline2D::SetVelocity( int pointIndex, const Vector2& newVelocity )
{
	m_velocities[pointIndex] = newVelocity;
}


void	CubicSpline2D::SetCardinalVelocities( float tension, const Vector2& startVelocity, const Vector2& endVelocity )
{
	for (size_t dataCount = 0; dataCount < m_velocities.size(); ++dataCount)
	{
		if (dataCount == 0)
		{
			m_velocities[dataCount] = startVelocity;
		}
		else if (dataCount == (m_velocities.size() - 1))
		{
			m_velocities[dataCount] = endVelocity;
		}
		else
		{
			m_velocities[dataCount] = (1 - tension) * (m_positions[dataCount + 1] - m_positions[dataCount - 1]) / 2.f;
		}
	}
}


const Vector2	CubicSpline2D::GetPosition( int pointIndex )
{
	return m_positions[pointIndex];
}


const Vector2	CubicSpline2D::GetVelocity( int pointIndex )
{
	return m_velocities[pointIndex];
}


int				CubicSpline2D::GetPositions( std::vector<Vector2>& out_positions ) const
{
	out_positions = m_positions;
	return (int) out_positions.size();
}


int				CubicSpline2D::GetVelocities( std::vector<Vector2>& out_velocities ) const 
{
	out_velocities = m_velocities;
	return (int) out_velocities.size();
}


Vector2			CubicSpline2D::EvaluateAtCumulativeParametric( float t ) const
{
	int startIndex = static_cast<int>(floor(t));
	int endIndex = startIndex + 1;
	Vector2 startPos = m_positions[startIndex];
	Vector2 startVel = m_velocities[startIndex];
	Vector2 endPos = m_positions[endIndex];
	Vector2 endVel = m_velocities[endIndex];

	float local_t = t - static_cast<float>(startIndex);

	Vector2 res = EvaluateCubicHermite(startPos, startVel, endPos, endVel, local_t);
	return res;
}


Vector2			CubicSpline2D::EvaluateAtNormalizedParametric( float t ) const
{
	int numCurve = GetNumPoints() - 1;
	float cumulative_t = RangeMapFloat(t, 0.f, 1.f, 0.f, static_cast<float>(numCurve) * 1.f);
	return EvaluateAtCumulativeParametric(cumulative_t);
}