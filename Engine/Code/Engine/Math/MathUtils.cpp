#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Transform.hpp"
#include "Engine/Core/Profiler/ProfileSystem.hpp"

float ConvertDegreesToRadians(float degrees)
{
	return degrees * (PI / 180.f);
}


float ConvertRadiansToDegrees(float radians)
{
	return radians * (180.f / PI);
}


float CosDegrees(float degrees)
{
	return cosf(ConvertDegreesToRadians(degrees));
}


float SinDegrees(float degrees)
{
	return sinf(ConvertDegreesToRadians(degrees));
}


float GetRandomFloatInRange(float minInclusive, float maxInclusive)
{
	return (GetRandomFloatZeroToOne() * (maxInclusive - minInclusive)) + minInclusive;
}


float GetRandomFloatZeroToOne()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}


int GetRandomIntInRange(int minInclusive, int maxInclusive)
{
	return GetRandomIntLessThan(maxInclusive - minInclusive + 1) + minInclusive;
}


int GetRandomIntLessThan(int maxNotInclusive)
{
	return rand() % maxNotInclusive;
}

Vector3 GetRandomVector3(const Vector3& v1, const Vector3& v2)
{
	float rand_x = GetRandomFloatInRange(v1.x, v2.x);
	float rand_y = GetRandomFloatInRange(v1.y, v2.y);
	float rand_z = GetRandomFloatInRange(v1.z, v2.z);
	return Vector3(rand_x, rand_y, rand_z);
}

float GetDistance(const Vector2& a, const Vector2& b)
{
	float newX = b.x - a.x;
	float newY = b.y - a.y;

	return Vector2(newX, newY).GetLength();
}

float GetDistanceSquared(const Vector2& a, const Vector2& b)
{
	return GetDistance(a, b) * GetDistance(a, b);
}


bool DoDiscsOverlap (const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius)
{
	float discsDistance = GetDistance(aCenter, bCenter);
	
	return (discsDistance <= (aRadius + bRadius));
}


bool DoDiscsOverlap (const Disc2& a, const Disc2& b)
{
	return DoDiscsOverlap(a.center, a.radius, b.center, b.radius);
}


bool DoAABBsOverlap( const AABB2& a, const AABB2& b )
{
	Vector2 aCenter = a.GetCenter();
	Vector2 bCenter = b.GetCenter();
	Vector2 aDimension = a.GetDimensions();
	Vector2 bDimension = b.GetDimensions();

	bool overlapX = abs(aCenter.x - bCenter.x) * 2 <= (aDimension.x + bDimension.x);
	bool overlapY = abs(aCenter.y - bCenter.y) * 2 <= (aDimension.y + bDimension.y);

	return ( overlapX && overlapY );
}


float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
	if (inStart == inEnd)
	{
		return ( outStart + outEnd ) * 0.5f;
	}

	float inRange = inEnd - inStart;
	float outRange = outEnd - outStart;
	float inRelativeToStart = inValue - inStart;
	float fractionIntoRange = inRelativeToStart / inRange;
	float outRelativeToStart = outRange * fractionIntoRange;

	return (outStart + outRelativeToStart);
}


float ClampZeroToOne(float value)
{
	if (value > 1.f)
	{
		return 1.f;
	}
	else if (value < 0.f)
	{
		return 0.f;
	}

	return value;
}


float ClampZeroToFloat(float valueClampTo, float value)
{
	if (value > valueClampTo)
	{
		return valueClampTo;
	}
	else if (value < 0.f)
	{
		return 0.f;
	}

	return value;
}


bool CheckRandomChance( float chanceForSuccess )
{
	float random = GetRandomFloatInRange(0.0f, 1.0f);

	if (random <= chanceForSuccess)
	{
		return true;
	}

	return false;
}


float GetAngularDisplacement( float startDegrees, float endDegrees )
{
	float rawDisplacement = startDegrees > endDegrees ? startDegrees - endDegrees : endDegrees - startDegrees;
	float modDisplacement = fmod(rawDisplacement, 360.f);
	float displacement = ( modDisplacement > 180.f ) ? ( 360.f - modDisplacement ) : modDisplacement;

	float rawStart = fmod(startDegrees, 360.f);
	float rawEnd = fmod(endDegrees, 360.f);

	float positiveStart;
	float positiveEnd;

	if (rawStart < 0.f)
	{
		positiveStart = 360.f + rawStart;
	}
	else 
	{
		positiveStart = rawStart;
	}

	if (rawEnd < 0.f)
	{
		positiveEnd = 360.f + rawEnd;
	}	
	else 
	{
		positiveEnd = rawEnd;
	}

	int direction;

	if (positiveStart > positiveEnd)
	{
		if (positiveStart > positiveEnd + 180.f)
		{
			direction = 1;
		}

		else
		{
			direction = -1;
		}
	}

	else
	{
		if (positiveStart < positiveEnd - 180.f)
		{
			direction = -1;
		}

		else
		{
			direction = 1;
		}
	}

	return direction * displacement;
}


float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees )
{
	float displacement = GetAngularDisplacement(currentDegrees, goalDegrees);

	if (abs(displacement) > maxTurnDegrees)
	{
		if (displacement > 0.f)
		{
			displacement = maxTurnDegrees;
		}
		else 
		{
			displacement = -maxTurnDegrees;
		}

		return currentDegrees + displacement;
	}

	return goalDegrees;
}


float DotProduct( const Vector2& a, const Vector2& b )
{
	return (a.x * b.x) + (a.y * b.y);
}


float DotProduct( const Vector3& a, const Vector3& b )
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}


float DotProduct( const Vector4& a, const Vector4& b )
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}


int RoundToNearestInt( float inValue )
{
	if ( ( fmod(inValue, 0.5f) == 0.f ) && ( fmod(inValue, 1.f) != 0.f ) )
	{
		return static_cast<int>( (inValue + 0.5f) );
	}
	else 
	{
		return static_cast<int>( roundf(inValue) );
	}
}


float RoundToNearest05(float inValue)
{
	float res = 0.f;

	if ( ( fmod(inValue, 0.5f) == 0.f ) && ( fmod(inValue, 1.f) == 0.f ) )
	{
		res = inValue - 0.5f;
	}
	else if ( ( fmod(inValue, 0.5f) == 0.f ) && ( fmod(inValue, 1.f) == 0.5f ) )
	{
		res = inValue;
	}
	else if ( ( fmod(inValue, 0.5f) != 0.f ) && ( fmod(inValue, 1.f) < 0.5f ) )
	{
		float mod1 = fmod(inValue, 1.f);
		float offset = 0.5f - mod1;
		res = inValue + offset;
	}
	else if ( ( fmod(inValue, 0.5f) != 0.f ) && ( fmod(inValue, 1.f) > 0.5f ) )
	{
		float mod1 = fmod(inValue, 1.f);
		float offset = mod1 - 0.5f;
		res = inValue - offset;
	}

	return res;
}



int ClampInt( int inValue, int min, int max )
{
	if (inValue > max)
	{
		return max;
	}
	else if (inValue < min)
	{
		return min;
	}
	else 
	{
		return inValue;
	}
}


float ClampFloat( float inValue, float min, float max )
{
	if (inValue > max)
	{
		return max;
	}
	else if (inValue < min)
	{
		return min;
	}
	else
	{
		return inValue;
	}
}


float ClampFloatNegativeOneToOne( float inValue )
{
	if (inValue < -1.f)
	{
		return -1.f;
	}
	else if (inValue > 1.f)
	{
		return 1.f;
	}
	else
	{
		return inValue;
	}
}


float GetFractionInRange( float inValue, float rangeStart, float rangeEnd )
{
	return (inValue - rangeStart) / (rangeEnd - rangeStart);
}


bool AreBitsSet( unsigned char bitFlags8, unsigned char flagsToCheck )
{
	unsigned char res = bitFlags8 & flagsToCheck;

	if (res == flagsToCheck)
	{
		return true;
	}

	return false;
}


bool AreBitsSet( unsigned int bitFlags32, unsigned int flagsToCheck )
{
	unsigned int res = bitFlags32 & flagsToCheck;

	if (res == flagsToCheck)
	{
		return true;
	}

	return false;
}


void SetBits( unsigned char& bitFlags8, unsigned char flagsToSet )
{
	bitFlags8 = bitFlags8 | flagsToSet;
}


void SetBits( unsigned int& bitFlags32, unsigned int flagsToSet )
{
	bitFlags32 = bitFlags32 | flagsToSet;
}


void ClearBits( unsigned char& bitFlags8, unsigned char flagToClear )
{
	unsigned char reverseMask = ~(flagToClear);
	bitFlags8 = bitFlags8 & reverseMask;
}


void ClearBits( unsigned int& bitFlags32, unsigned int flagToClear )
{
	unsigned int reverseMask = ~(flagToClear);
	bitFlags32 = bitFlags32 & reverseMask;
}


int CheckBit(int& subject, int bitPos)
{
	return (subject) & (1 << bitPos);
}

void RightShiftLostBit(std::vector<uint8_t>& bits, size_t* num, int shiftNum)
{
	for (int i = 0; i < shiftNum; ++i)
	{
		uint8_t shiftedBit = RightShiftLostBitUnit(num);
		bits.push_back(shiftedBit);
	}

	std::reverse(bits.begin(), bits.end());
}

void RightShiftRemainedBit(std::vector<uint8_t>& remained, size_t* num, int shiftNum)
{
	std::vector<uint8_t> bits = BinaryFromDecimal(*num);
	int bits_num = (int)bits.size();
	int bits_remained = bits_num - shiftNum;

	while (bits_num > 0)
	{
		uint8_t lost_bit = RightShiftLostBitUnit(num);
		if (bits_num <= bits_remained)
			remained.push_back(lost_bit);
		bits_num--;
	}

	std::reverse(remained.begin(), remained.end());
}

uint8_t RightShiftLostBitUnit(size_t* num)
{
	bool lost_bit = (*num) & 1;
	*num >>= 1;
	if (lost_bit)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void SetHighBit(std::vector<uint8_t>& subject, uint8_t zero_one)
{
	// get the iterator first
	std::vector<uint8_t>::iterator it;
	// get the beginning iterator then
	it = subject.begin();

	// we want to add zero_one to the head of vector, so we use the iterator
	subject.insert(it, zero_one);
}

size_t DecimalFromBinary(std::vector<uint8_t>& binary)
{
	int decimal = 0;
	int binarySize = (int)binary.size();

	for (int i = binarySize - 1; i >= 0; --i)
	{
		uint8_t bit = binary[i];

		// power: binarySize - i - 1
		int power = binarySize - i - 1;

		int increment = bit * (int)pow(2, power);

		// increment the contribution from each bit to the final res of decimal
		decimal += increment;
	}

	return (size_t)decimal;
}

std::vector<uint8_t> BinaryFromDecimal(size_t decimal)
{
	std::vector<uint8_t> res;
	res.reserve(8);
	for (int i = 0; i < 8; ++i)
		res.push_back(0);

	// counter for binary
	int j = 7;
	while (decimal > 0U) 
	{
		// storing remainder in binary array
		res[j] = decimal % 2;
		decimal /= 2;
		j--;
	}

	return res;
}

std::vector<uint8_t> BinaryFromDecimal16(size_t decimal)
{
	std::vector<uint8_t> res;
	res.reserve(16);
	for (int i = 0; i < 16; ++i)
		res.push_back(0);

	// counter for binary
	int j = 15;
	while (decimal > 0U) 
	{
		// storing remainder in binary array
		res[j] = decimal % 2;
		decimal /= 2;
		j--;
	}

	return res;
}

uint16_t BufferSizeHardCode16(std::string str)
{
	uint16_t code = 65535U;

	if (str == "\x0") code = 0;
	if (str == "\x1") code = 1;
	if (str == "\x2") code = 2;
	if (str == "\x3") code = 3;
	if (str == "\x4") code = 4;
	if (str == "\x5") code = 5;
	if (str == "\x6") code = 6;
	if (str == "\x7") code = 7;
	if (str == "\x8") code = 8;
	if (str == "\x9") code = 9;
	if (str == "\x10") code = 10;
	if (str == "\x11") code = 11;
	if (str == "\x12") code = 12;
	if (str == "\x13") code = 13;
	if (str == "\x14") code = 14;
	if (str == "\x15") code = 15;
	if (str == "\x16") code = 16;
	if (str == "\x17") code = 17;
	if (str == "\x18") code = 18;
	if (str == "\x19") code = 19;
	if (str == "\x20") code = 20;
	return code;
}

float	SmoothStart2( float t )
{
	return t * t;
}


float	SmoothStart3( float t )
{
	return t * t * t;
}


float	SmoothStart4( float t )
{
	return t * t * t * t;
}


float	SmoothStop2( float t )
{
	return 1 - ((1 - t) * (1 - t));
}


float	SmoothStop3( float t )
{
	return 1 - ((1 - t) * (1 - t) * (1 - t));
}


float	SmoothStop4( float t )
{
	return 1 - ((1 - t) * (1 - t) * (1 - t) * (1 - t));
}


float	SmoothStep3( float t )
{
	return SmoothStart2(t) + t * (SmoothStop2(t) - SmoothStart2(t));
}


float Interpolate( float start, float end, float fractionTowardEnd )
{
	return (start + ((end - start) * fractionTowardEnd));
}


Vector3 Interpolate(Vector3 start, Vector3 end, float fractionTowardEnd)
{
	return start + (end - start) * fractionTowardEnd;
}


int Interpolate( int start, int end, float fractionTowardEnd )
{
	int diff = end - start;
	int increment;
	if (fmod(fractionTowardEnd * diff, 1.f) >= 0.5f)
	{
		increment = static_cast<int>(fractionTowardEnd * diff) + 1;
	}
	else
	{
		increment = static_cast<int>(fractionTowardEnd * diff);
	}
	return start + increment;
}


unsigned char Interpolate( unsigned char start, unsigned char end, float fractionTowardEnd )
{
	unsigned char diff = end - start;
	unsigned char increment;
	if (fmod(fractionTowardEnd * diff, 1.f) >= 0.5f)
	{
		increment = static_cast<unsigned char>(fractionTowardEnd * diff) + 1;
	}
	else
	{
		increment = static_cast<unsigned char>(fractionTowardEnd * diff);
	}

	return start + increment;
}


std::vector<int> SetFromText(const char* text)
{
	std::vector<int> intVec;

	std::string str(text);
	while (str.find(',') != std::string::npos)
	{
		intVec.push_back(atoi(str.c_str()));
		str = str.substr(str.find(',') + 1);
	}

	intVec.push_back(atoi(str.c_str()));
	
	return intVec;
}


Vector2 PolarToCartesian(float radius, float degree)
{
	float x = radius * CosDegrees(degree);
	float y = radius * SinDegrees(degree);

	return Vector2(x, y);
}


Vector3 PolarToCartesian( float radius, float rotationDeg, float azimuthDeg )
{
	Vector3 res;

	res.x = radius * SinDegrees(azimuthDeg) * CosDegrees(rotationDeg);
	res.y = radius * CosDegrees(azimuthDeg);
	res.z = radius * SinDegrees(azimuthDeg) * SinDegrees(rotationDeg);
	
	return res;
}


Vector3 PolarToCartesian( Vector3 spherical )
{
	Vector3 res;

	res = PolarToCartesian(spherical.x, spherical.y, spherical.z);

	return res;
}


Vector3 CartesianToPolar( Vector3 position )
{
	Vector3 res;

	float x = position.x;
	float y = position.y;
	float z = position.z;

	res.x = ConvertRadiansToDegrees(sqrtf(x * x + y * y + z * z));		// radius
	res.y = ConvertRadiansToDegrees(atan2f(z, x));						// rotation, radian
	res.z = ConvertRadiansToDegrees(atan2f(sqrtf(x * x + z * z), y));	// azimuth, radian

	return res;
}

// assume x is normalized 
void MakeOrthonormalBasis(const Vector3& x, Vector3& y, Vector3& z)
{
	z = x.Cross(y);

	if (z.GetLengthSquared() == 0.f)
		return;

	y = z.Cross(x);

	// normalize
	y.Normalize();
	z.Normalize();
}

void MakeOrthonormalBasisOpt(const Vector3& x, Vector3& y, Vector3& z)
{
	const float s = 1.f / sqrtf(x.z * x.z + x.x * x.x);
	
	// z
	z.x = x.z * s;
	z.y = 0.f;
	z.z = -x.x * s;

	// y
	y.x = x.y * z.x;
	y.y = x.z * z.x - x.x * z.z;
	y.z = -x.y * z.x;
}

/*
void MakeOrthonormalBasisStable(const Vector3& x)
{
	if (abs(x.x) > abs(x.y))
	{
		// near x axis, suggest y axis
		Vector3 y = Vector3(0.f, 1.f, 0.f);
		Vector3 z;
		MakeOrthonormalBasisOpt(x, y, z);
	}
	else
	{
		// near y axis, suggest x axis
		Vector3 y = Vector3(1.f, 0.f, 0.f);
		Vector3 z;
		MakeOrthonormalBasisOpt(x, y, z);
	}
}
*/

bool ProjectPlaneToSphere(Vector2 pos, float r, Vector3& out_pos)
{
	bool inCircle = IsPointInCircle(pos, r);
	if (!inCircle)
		return inCircle;

	float toOriginSq = pos.GetLengthSquared();
	float radSq = r * r;
	float height = sqrtf(radSq - toOriginSq);
	out_pos.x = pos.x;
	out_pos.y = height;
	out_pos.z = pos.y;
	return inCircle;
}

bool Quadratic(Vector2& out, float a, float b, float c)
{
	float delta = b * b - 4 * a * c;
	float numerator_0 = -b - sqrtf(delta);
	float numerator_1 = -b + sqrtf(delta);
	float denominator = 2 * a;

	if (denominator == 0)
		return false;

	out.x = numerator_0 / denominator;
	out.y = numerator_1 / denominator;

	return true;
}


int ComputeManhattanDistanceInt(IntVector2 v1, IntVector2 v2)
{
	int x = abs(v1.x - v2.x);
	int y = abs(v1.y - v2.y);

	return (x + y);
}


float ComputeManhattanDistanceFloat(Vector2 v1, Vector2 v2)
{
	float x = abs(v1.x - v2.x);
	float y = abs(v1.y - v2.y);

	return (x + y);
}


// v1, v2 and v3 in CCW order
float ComputeTriangleArea(Vector3 v1, Vector3 v2, Vector3 v3)
{
	Vector3 side_1 = v2 - v1;
	Vector3 side_2 = v3 - v1;

	Vector3 cross = side_2.Cross(side_1);
	float parallelogramArea = cross.GetLength();

	float triangleArea = parallelogramArea / 2.f;

	return triangleArea;
}


// v1, v2 and v3 in CCW order
Vector3 ComputeTriangleCenter(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 center;

	float averageCoef = 1.f / 3.f;

	float coef_v3_v1_center = averageCoef;
	float coef_v1_v2_center = averageCoef;
	float coef_v2_v3_center = 1 - coef_v3_v1_center - coef_v1_v2_center;

	center = (v1 * coef_v3_v1_center) + (v2 * coef_v1_v2_center) + (v3 * coef_v2_v3_center);

	return center;
}


Vector3 Slerp(const Vector3& v1, const Vector3& v2, float fraction)
{
	float len1 = v1.GetLength();
	float len2 = v2.GetLength();

	float len = Interpolate(len1, len2, fraction);
	Vector3 res = SlerpUnit( v1 / len1, v2 / len2, fraction ); 
	return res * len;
}


Vector3 SlerpUnit(const Vector3& v1, const Vector3& v2, float fraction)
{
	float cosAngle = ClampFloat(DotProduct(v1, v2), -1.0f, 1.0f);
	float angle = ConvertRadiansToDegrees(acosf(cosAngle));
	if (angle < SLERP_ANGLE_THRESHOLD)
	{
		return Interpolate( v1, v2, fraction );
	}
	else 
	{
		float pos_num = sinf( fraction * angle );
		float neg_num = sinf( (1.0f - fraction) * angle );
		float den = sinf(angle);

		return v1 * (neg_num / den) + v2 * (pos_num / den);
	}
}


Matrix44 LerpTransform(const Matrix44& m1, const Matrix44& m2, float fraction)
{
	Vector3 m1_right = m1.GetRight();
	Vector3 m2_right = m2.GetRight(); 
	Vector3 m1_up = m1.GetUp();
	Vector3 m2_up = m2.GetUp(); 
	Vector3 m1_forward = m1.GetForward(); 
	Vector3 m2_forward = m2.GetForward();
	Vector3 m1_translation = m1.GetTranslation();
	Vector3 m2_translation = m2.GetTranslation(); 

	Vector3 right = Slerp( m1_right, m2_right, fraction ); 
	Vector3 up = Slerp( m1_up, m2_up, fraction ); 
	Vector3 forward = Slerp( m1_forward, m2_forward, fraction ); 
	Vector3 translation = Interpolate( m1_translation, m2_translation, fraction ); 

	return Matrix44::FromBasisTranslation(right, up, forward, translation);
}


void SwapFloat(float& t1, float& t2)
{
	float tempf = t1;

	t1 = t2;
	t2 = tempf;
}

bool IsIdenticalFloat(float f1, float f2)
{
	if (abs(f1 - f2) <= 0.01f)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

bool IsIdenticalVector3(Vector3 v1, Vector3 v2)
{
	float distance = (v1 - v2).GetLength();

	if (distance <= 0.01f)
	{
		return true;
	}
	else
	{
		return false;
	}
}

float GetSquredDistPointVsAABB3(const Vector3& point, const AABB3& aabb3)
{
	float squaredDistance = 0.f;

	float pointX = point.x;
	float pointY = point.y;
	float pointZ = point.z;

	float xSquaredDistance = 0.f;
	float ySquaredDistance = 0.f;
	float zSquaredDistance = 0.f;

	// x test
	float xMin = aabb3.m_min.x;
	float xMax = aabb3.m_max.x;
	if (pointX < xMin)
	{
		float distance = xMin - pointX;
		xSquaredDistance = distance * distance;
	}
	else if (pointX > xMax)
	{
		float distance = pointX - xMax;
		xSquaredDistance = distance * distance;
	}

	// y test
	float yMin = aabb3.m_min.y;
	float yMax = aabb3.m_max.y;
	if (pointY < yMin)
	{
		float distance = yMin - pointY;
		ySquaredDistance = distance * distance;
	}
	else if (pointY > yMax)
	{
		float distance = pointY - yMax;
		ySquaredDistance = distance * distance;
	}

	// z test
	float zMin = aabb3.m_min.z;
	float zMax = aabb3.m_max.z;
	if (pointZ < zMin)
	{
		float distance = zMin - pointZ;
		zSquaredDistance = distance * distance;
	}
	else if (pointZ > zMax)
	{
		float distance = pointZ - zMax;
		zSquaredDistance = distance * distance;
	}

	squaredDistance = xSquaredDistance + ySquaredDistance + zSquaredDistance;

	return squaredDistance;
}

bool SphereVsAABB3Intersection(const Sphere3& sphere, const AABB3& aabb3)
{
	Vector3 center = sphere.m_center;
	float radius = sphere.m_radius;

	float squredSphereDistToAABB3 = GetSquredDistPointVsAABB3(center, aabb3);

	bool intersected = (squredSphereDistToAABB3 <= (radius * radius));
	
	return intersected;
}

bool SphereVsSphereIntersection(const Sphere3& s1, const Sphere3& s2)
{
	Vector3 center1 = s1.m_center;
	Vector3 center2 = s2.m_center;

	float radius1 = s1.m_radius;
	float radius2 = s2.m_radius;

	float distance = (center1 - center2).GetLength();
	float threshold = radius1 + radius2;

	bool intersected = (distance <= threshold);

	return intersected;
}

Vector3 TransformBasisPoint(const Vector3& point, const Matrix44& basis)
{
	Vector4 pointV4 = Vector4(point.x, point.y, point.z, 1.f);

	Vector4 transformedPointV4 = basis * pointV4;

	Vector3 transformedPoint = Vector3(transformedPointV4.x, transformedPointV4.y, transformedPointV4.z);

	return transformedPoint;
}

bool SphereVsOBB3Intersection(const Sphere3&, const OBB3&)
{
	/*
	Vector3 sphereCenter = sphere.m_center;
	float sphereRadius = sphere.m_radius;

	Vector3 obbCenter = obb3.m_center;
	Vector3 obbRot = obb3.m_rot;
	Vector3 obbScale = obb3.m_scale;

	Matrix44 rotMat = Matrix44::MakeRotationDegrees3D(obbRot);
	Vector3 transformedSphereCenter = TransformBasisPoint(sphereCenter, rotMat);
	Sphere3 transformedSphere = Sphere3(transformedSphereCenter, sphereRadius);

	Transform transform = Transform(obbCenter, obbRot, obbScale);
	Matrix44 localModel = transform.GetLocalMatrix();
	Vector3 localForwardNorm = localModel.GetForward().GetNormalized();
	Vector3 localRightNorm = localModel.GetRight().GetNormalized();
	Vector3 localUpNorm = localModel.GetUp().GetNormalized();

	Vector3 halfDim = Vector3(obbScale.x / 2.f, obbScale.y / 2.f, obbScale.z / 2.f);
	Vector3 halfExtX = localRightNorm * halfDim.x;
	Vector3 halfExtY = localUpNorm * halfDim.y;
	Vector3 halfExtZ = localForwardNorm * halfDim.z;
	
	Vector3 localMax = obbCenter + halfExtX + halfExtY + halfExtZ;
	Vector3 localMin = obbCenter - halfExtX - halfExtY - halfExtZ;

	Vector3 transformedMax = TransformBasisPoint(localMax, rotMat);
	Vector3 transformedMin = TransformBasisPoint(localMin, rotMat);
	
	AABB3 transformedOBB3 = AABB3(transformedMin, transformedMax);

	bool intersected = SphereVsAABB3Intersection(transformedSphere, transformedOBB3);

	return intersected;
	*/

	return false;
}

bool AABB3VsPlaneIntersection(const AABB3& aabb, const Plane& plane)
{
	float dist[8];
	Vector3 vertices[8];
	aabb.GetVertices(vertices);

	float planeOffset = plane.GetOffset();
	Vector3 planeNormal = plane.GetNormal();

	for (uint idx = 0; idx < 8; ++idx)
	{
		Vector3 vert = vertices[idx];

		float dot = DotProduct(vert, planeNormal);
		float signedDist = dot - planeOffset;

		dist[idx] = signedDist;
	}

	bool intersected = false;
	bool front = dist[0] > 0.f;		// forwardness/backwardness of first vert

	if (front)
	{
		for (uint idx = 1; idx < 8; ++idx)
		{
			if (dist[idx] <= 0.f)
			{
				intersected = true;
				break;
			}
		}
	}
	else
	{
		for (uint idx = 1; idx < 8; ++idx)
		{
			if (dist[idx] >= 0.f)
			{
				intersected = true;
				break;
			}
		}
	}

	return intersected;
}

bool AABB3VsAABB3Intersection(const AABB3& aabb_1, const AABB3& aabb_2)
{
	// Get all 15 axis to test with SAT.
	// 3 principal axes for aabb_1, 3 for aabb_2,
	// 9 for axes perpendicular to each pair of principal axes from each aabb.
	// However, in special case of aabb, there are a lot of overlap of those axes,
	// and all of them fall into the 3 principal axes of the space,
	// not considering 0 vector resulting from cross product of identical vectors.

	// x
	Vector3 x_axis = Vector3(1.f, 0.f, 0.f);
	bool x_overlap = OverlapOnAxis(aabb_1, aabb_2, x_axis);

	if (!x_overlap) return false;

	// y 
	Vector3 y_axis = Vector3(0.f, 1.f, 0.f);
	bool y_overlap = OverlapOnAxis(aabb_1, aabb_2, y_axis);

	if (!y_overlap) return false;

	// z
	Vector3 z_axis = Vector3(0.f, 0.f, 1.f);
	bool z_overlap = OverlapOnAxis(aabb_1, aabb_2, z_axis);

	if (!z_overlap) return false;

	return true;
}

/*
 * Returns if two boxes overlap.
 * @param overlap: amount overlapped. axis: overlap axis, for aabbs, it is either x, y or z
 */
// UNTESTED
bool AABB3VsAABB3Intersection(const AABB3& aabb_1, const AABB3& aabb_2, Vector3& axis, float& overlap)
{
	Vector3 x_axis = Vector3(1.f, 0.f, 0.f);
	float x_overlap;
	bool x_overlapped = OverlapOnAxis(aabb_1, aabb_2, x_axis, x_overlap);

	if (!x_overlapped)
	{
		axis = Vector3::ZERO;
		overlap = 0;
		return false;
	}

	Vector3 y_axis = Vector3(0.f, 1.f, 0.f);
	float y_overlap;
	bool y_overlapped = OverlapOnAxis(aabb_1, aabb_2, y_axis, y_overlap);

	if (!y_overlapped)
	{
		axis = Vector3::ZERO;
		overlap = 0;
		return false;
	}

	Vector3 z_axis = Vector3(0.f, 0.f, 1.f);
	float z_overlap;
	bool z_overlapped = OverlapOnAxis(aabb_1, aabb_2, z_axis, z_overlap);

	if (!z_overlapped)
	{
		axis = Vector3::ZERO;
		overlap = 0;
		return false;
	}

	// two aabb3s overlap
	float min_overlap = INFINITY;
	if (x_overlap < min_overlap)
	{
		axis = Vector3(1.f, 0.f, 0.f);
		min_overlap = x_overlap;
	}
	if (y_overlap < min_overlap)
	{
		axis = Vector3(0.f, 1.f, 0.f);
		min_overlap = y_overlap;
	}
	if (z_overlap < min_overlap)
	{
		axis = Vector3(0.f, 0.f, 1.f);
		min_overlap = z_overlap;
	}

	overlap = min_overlap;
	return true;
}

AABB2 MinkowskiAABBVsAABB(const AABB2& aabb1, const AABB2& aabb2)
{
	// minkowski difference of two aabbs is still an aabb
	Vector2 min1 = aabb1.mins;
	Vector2 max2 = aabb2.maxs;
	Vector2 bottomLeft = min1 - max2;
	Vector2 size = aabb1.GetDimensions() + aabb2.GetDimensions();
	Vector2 topRight = bottomLeft + size;
	return AABB2(bottomLeft, topRight);
}

bool IsPointInCircle(Vector2 pos, float r)
{
	return (pos.GetLength() <= r);
}

float DistPointToEdge(const Vector3& pt, const Vector3& vert1, const Vector3& vert2)
{
	Vector3 numerator = (pt - vert1).Cross(pt - vert2);
	Vector3 denominator = vert2 - vert1;

	float numeratorF = numerator.GetLength();
	float denominatorF = denominator.GetLength();

	return (numeratorF / denominatorF);
}

float DistPointToPlaneSigned(const Vector3& pt, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3)
{
	// use Hessian normal form, see http://mathworld.wolfram.com/Point-PlaneDistance.html
	Vector3 norm_numerator = (vert2 - vert1).Cross(vert3 - vert1);
	float norm_denom = norm_numerator.GetLength();
	Vector3 unit_norm = norm_numerator / norm_denom;

	// vert1/vert2/ver3 is fine here
	float signed_dist = DotProduct(unit_norm, pt - vert1);
	
	return signed_dist;
}

float DistPointToPlaneUnsigned(const Vector3& pt, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3)
{
	return abs(DistPointToPlaneSigned(pt, vert1, vert2, vert3));
}

/*
* Return the closest feature on convex hull regarding a point.
* @param pt: position of point of interest
* @param a: a vertex of the triangle
* @param b: b vertex of the triangle
* @param c: c vertex of the triangle
* @param dist: reference to the closest distance
* @param closest: reference to the closest point 
* @return the closest feature
*/
QHFeature* DistPointToTriangleHull(const Vector3& pt, const Vector3& a, 
	const Vector3& b, const Vector3& c, float& dist, Vector3& closest)
{
	// see p139 of real time collision detection
	// in my case, I want the distance and feature info
	Vector3 ab = b - a;
	Vector3 ac = c - a;
	Vector3 bc = c - b;

	// Voronoi related to ab
	float snom = DotProduct(pt - a, ab);
	float sdenom = DotProduct(pt - b, a - b);

	// Voronoi related to ac
	float tnom = DotProduct(pt - a, ac);
	float tdenom = DotProduct(pt - c, a - c);

	if (snom <= 0.f && tnom <= 0.f)
	{
		closest = a;
		dist = (pt - a).GetLength();
		QHVert* feature = new QHVert(a);

		return feature;
	}

	// Voronoi related to bc
	float unom = DotProduct(pt - b, bc);
	float udenom = DotProduct(pt - c, b - c);
	
	if (sdenom <= 0.f && unom <= 0.f)
	{
		closest = b;
		dist = (pt - b).GetLength();
		QHVert* feature = new QHVert(b);

		return feature;
	}

	if (tdenom <= 0.f && udenom <= 0.f)
	{
		closest = c;
		dist = (pt - c).GetLength();
		QHVert* feature = new QHVert(c);
		
		return feature;
	}

	// investigate edge features with barycentric methods
	// ab
	Vector3 n = ab.Cross(ac);
	Vector3 toA = a - pt;
	Vector3 toB = b - pt;
	float vc = DotProduct(n, toA.Cross(toB));
	if (vc <= 0.f && snom >= 0.f && sdenom >= 0.f)
	{
		// closest feature is edge ab
		Vector3 dev = ab * (snom / (snom + sdenom));
		closest = a + dev;
		dist = (-toA - dev).GetLength();
		QHEdge* feature = new QHEdge(a, b);

		return feature;
	}

	// bc
	Vector3 toC = c - pt;
	float va = DotProduct(n, toB.Cross(toC));
	if (va <= 0.f && unom >= 0.f && udenom >= 0.f)
	{
		// closest feature is edge bc
		Vector3 dev = bc * (unom / (unom + udenom));
		closest = b + dev;
		dist = (-toB - dev).GetLength();
		QHEdge* feature = new QHEdge(b, c);

		return feature;
	}

	// ac
	float vb = DotProduct(n, toC.Cross(toA));
	if (vb <= 0.f && tnom >= 0.f && tdenom >= 0.f)
	{
		// closest feature is edge ac
		Vector3 dev = ac * (tnom / (tnom + tdenom));
		closest = a + dev;
		dist = (-toA - dev).GetLength();
		QHEdge* feature = new QHEdge(a, c);

		return feature;
	}

	// in this case pt project within the triangle 
	float u = va / (va + vb + vc);
	float v = vb / (va + vb + vc);
	float w = 1.f - u - v;
	closest = a * u + b * v + c * w;
	dist = (pt - closest).GetLength();
	QHFace* feature = new QHFace(a, b, c);

	return feature;
}

QHFeature* DistPointToQuadHull(const Vector3& pt, const Vector3& vert1, 
	const Vector3& vert2, const Vector3& vert3, const Vector3& vert4,
	float& dist, Vector3& closest)
{
	// 1-2-3
	float dist123;
	Vector3 closest123;
	QHFeature* feature123 = DistPointToTriangleHull(pt, vert1, vert2, vert3, dist123, closest123);

	// 1-3-4
	float dist134;
	Vector3 closest134;
	QHFeature* feature134 = DistPointToTriangleHull(pt, vert1, vert3, vert4, dist134, closest134);

	if (dist123 < dist134)
	{
		closest = closest123;
		dist = dist123;
		return feature123;
	}
	else
	{
		closest = closest134;
		dist = dist134;
		return feature134;
	}
}
