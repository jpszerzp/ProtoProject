#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <math.h>


const Vector2 Vector2::ZERO = Vector2(0.f, 0.f);
const Vector2 Vector2::ONE = Vector2(1.f, 1.f);


//-----------------------------------------------------------------------------------------------
Vector2::Vector2( const Vector2& copy )
	: x( copy.x )
	, y( copy.y )
{

}


//-----------------------------------------------------------------------------------------------
Vector2::Vector2( float initialX, float initialY )
	: x( initialX )
	, y( initialY )
{

}


Vector2::Vector2(const float& initialXY)
	: x(initialXY)
	, y(initialXY)
{

}

//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator + ( const Vector2& vecToAdd ) const
{
	return Vector2( x + vecToAdd.x, y + vecToAdd.y); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator-( const Vector2& vecToSubtract ) const
{
	return Vector2( x - vecToSubtract.x, y - vecToSubtract.y); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator*( float uniformScale ) const
{
	return Vector2( x * uniformScale, y * uniformScale); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector2 Vector2::operator/( float inverseScale ) const
{
	return Vector2( x / inverseScale, y / inverseScale); // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator+=( const Vector2& vecToAdd )
{
	x = x + vecToAdd.x; // #MP1Fixme
	y = y + vecToAdd.y; // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator-=( const Vector2& vecToSubtract )
{
	x = x - vecToSubtract.x; // #MP1Fixme
	y = y - vecToSubtract.y; // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator*=( const float uniformScale )
{
	x = x * uniformScale; // #MP1Fixme
	y = y * uniformScale; // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator/=( const float uniformDivisor )
{
	x = x / uniformDivisor; // #MP1Fixme
	y = y / uniformDivisor; // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
void Vector2::operator=( const Vector2& copyFrom )
{
	x = copyFrom.x; // #MP1Fixme
	y = copyFrom.y; // #MP1Fixme
}


//-----------------------------------------------------------------------------------------------
const Vector2 operator*( float uniformScale, const Vector2& vecToScale )
{
	return Vector2( uniformScale * vecToScale.x, uniformScale * vecToScale.y); // #MP1Fixme
}


const Vector2 Vector2::operator-() const
{
	Vector2 flipped;

	flipped.x = -x;
	flipped.y = -y;

	return flipped;
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator==( const Vector2& compare ) const
{
	if ((x == compare.x) && (y == compare.y))
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Vector2::operator!=( const Vector2& compare ) const
{
	return !((*this) == compare); // #MP1Fixme
}


float Vector2::GetLength() const 
{
	return sqrtf( (x * x ) + (y * y) );
}


float Vector2::GetLengthSquared() const
{
	return ( (x * x) + (y * y) );
}


float Vector2::NormalizeAndGetLength()
{
	float length = GetLength();

	float scale = 1.f / length;

	x = x * scale;
	y = y * scale;

	return length;
}

Vector2 Vector2::GetNormalized() const
{
	float length = GetLength();

	float scale = 1.f / length;

	float newX = x * scale;
	float newY = y * scale;

	return Vector2(newX, newY);
}

float Vector2::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees( atan2(y, x) );
}

float Vector2::GetOrientationDegrees0To360() const
{
	if (atan2(y, x) <= 0.f)
	{
		return 360.f + ConvertRadiansToDegrees(atan2(y, x));
	}
	else
	{
		return ConvertRadiansToDegrees(atan2(y, x));
	}
}

float Vector2::GetDistanceToLineSegment(Vector2 lineStart, Vector2 lineEnd) const
{
	Vector2 lineSeg = lineEnd - lineStart;

	Vector2 toPoint = Vector2(x - lineStart.x, y - lineStart.y);
	Vector2 projected = GetProjectedVector(toPoint, lineSeg);
	Vector2 vertical = toPoint - projected;

	return vertical.GetLength();
}

Vector2 Vector2::MakeDirectionAtDegrees(float degrees)
{
	return Vector2(CosDegrees(degrees), SinDegrees(degrees));
}

Quadrant Vector2::ConvertVec2ToQuad(Vector2 loc)
{
	float deg = loc.GetOrientationDegrees();
	Quadrant res = Vector2::ComputeQuadrantAtDegrees(deg);
	return res;
}

Quadrant Vector2::ComputeQuadrantAtDegrees(float degrees)
{
	Quadrant quad;
	Vector2 direction = MakeDirectionAtDegrees(degrees);
	if (direction.x > 0.f && direction.y > 0.f)
		quad = Quadrant::First;
	else if (direction.x > 0.f && direction.y < 0.f)
		quad = Quadrant::Fourth;
	else if (direction.x < 0.f && direction.y > 0.f)
		quad = Quadrant::Second;
	else if (direction.x < 0.f && direction.y < 0.f)
		quad = Quadrant::Third;
	else 
		quad = Quadrant::OnAxis;

	return quad;
}

Vector2 Vector2::ConvertQuadToVec2(Quadrant quad)
{
	Vector2 res;

	switch (quad)
	{
	case First:
		res = Vector2(1.f, 1.f);
		break;
	case Second:
		res = Vector2(-1.f, 1.f);
		break;
	case Third:
		res = Vector2(-1.f, -1.f);
		break;
	case Fourth:
		res = Vector2(1.f, -1.f);
		break;
	case OnAxis:
		break;
	case NUM_QUADRANT:
		break;
	default:
		break;
	}

	return res;
}

IntVector2 Vector2::ToIntVec2()
{
	IntVector2 intVec;

	intVec.x = static_cast<int>(x);
	intVec.y = static_cast<int>(y);

	return intVec;
}

Vector2 Vector2::GetRandomPositionVector2(float widthRange, float heightRange)
{
	float x = GetRandomFloatInRange(0.f, widthRange);
	float y = GetRandomFloatInRange(0.f, heightRange);

	return Vector2(x, y);
}

const Vector2 GetPerpendicularVector(const Vector2& vectorToProject, const Vector2& projectOnto)
{
	Vector2 projected = GetProjectedVector(vectorToProject, projectOnto);
	return (vectorToProject - projected);
}

const Vector2 GetProjectedVector( const Vector2& vectorToProject, const Vector2& projectOnto )
{
	Vector2 normalized = projectOnto.GetNormalized();
	float length = DotProduct(vectorToProject, normalized);

	return length * normalized;
}

const Vector2 GetTransformedIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ )
{
	float iLength = DotProduct(originalVector, newBasisI);
	float jLength = DotProduct(originalVector, newBasisJ);
		
	return Vector2(iLength, jLength);
}

const Vector2 GetTransformedOutOfBasis( const Vector2& vectorInBasis, const Vector2& oldBasisI, const Vector2& oldBasisJ )
{
	return (vectorInBasis.x * oldBasisI + vectorInBasis.y * oldBasisJ);
}

void DecomposeVectorIntoBasis( const Vector2& originalVector, const Vector2& newBasisI, const Vector2& newBasisJ, Vector2& out_vectorAlongI, Vector2& out_vectorAlongJ )
{
	out_vectorAlongI = GetProjectedVector(originalVector, newBasisI);
	out_vectorAlongJ = GetProjectedVector(originalVector, newBasisJ);
}

const Vector2 Interpolate( const Vector2& start, const Vector2& end, float fractionTowardEnd )
{
	float x = Interpolate(start.x, end.x, fractionTowardEnd);
	float y = Interpolate(start.y, end.y, fractionTowardEnd);

	return Vector2(x, y);
}

const Vector2 Reflect(const Vector2& inVec, const Vector2& normal)
{
	Vector2 projected = GetProjectedVector(inVec, normal);
	return inVec - 2 * projected;
}

void Vector2::SetFromText( const char* text )
{
	const char* comma = strchr(text, ',');
	
	x = static_cast<float>(atof(text));
	y = static_cast<float>(atof(comma + 1));
}


const std::string ToString(const Vector2& vector)
{
	std::string vecStr = "(";
	vecStr += std::to_string(vector.x);
	vecStr += ", ";
	vecStr += std::to_string(vector.y);
	vecStr += ")";

	return vecStr;
}


const float CrossProduct2(Vector2 p1, Vector2 p2)
{
	return p1.x * p2.y - p1.y * p2.x;
}

Vector3 Vector2::ToVector3(float z)
{
	return Vector3(x, y, z);
}


float Vector2::ComputeDegOfVector2s(Vector2& a, Vector2& b)
{
	Vector2 aNomralized = a.GetNormalized();
	Vector2 bNormalized = b.GetNormalized();

	float dot = DotProduct(aNomralized, bNormalized);
	float det = aNomralized.x * bNormalized.y - aNomralized.y * bNormalized.x;

	float angle = ConvertRadiansToDegrees(atan2(det, dot));
	return angle;
}


Vector2 Vector2::RotateDegree2D(float deg)
{
	float orientationDeg = this->GetOrientationDegrees0To360();
	float newDeg = orientationDeg + deg;
	float length = GetLength();

	Vector2 unitVec;

	float newX = CosDegrees(newDeg);
	float newY = SinDegrees(newDeg);

	unitVec.x = 1.f * newX;
	unitVec.y = 1.f * newY;

	return (unitVec * length);
}