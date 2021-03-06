#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Transform.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

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

Vector3 GetRandomVector3()
{
	float rand_x = GetRandomFloatInRange(-1.f, 1.f);
	float rand_y = GetRandomFloatInRange(-1.f, 1.f);
	float rand_z = GetRandomFloatInRange(-1.f, 1.f);
	Vector3 res = Vector3(rand_x, rand_y, rand_z);
	return res.GetNormalized();
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

Vector3 ProjectPointToPlane(const Vector3& point, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, float& dist)
{
	Vector3 norm_numerator = (vert2 - vert1).Cross(vert3 - vert1);
	float norm_denom = norm_numerator.GetLength();
	Vector3 unit_norm = norm_numerator / norm_denom;

	Vector3 disp = point - vert1;
	float signed_dist = DotProduct(unit_norm, disp);
	dist = abs(signed_dist);			// unsigned distance
	Vector3 vertical = unit_norm * signed_dist;
	Vector3 horizontal = disp - vertical;

	// projection - vert1 = horizontal
	Vector3 projection = horizontal + vert1;

	return projection;
}

Vector3 ProjectPointToPlaneFeature(const Vector3& pt, const Vector3& a, const Vector3& b, const Vector3& c, float& dist)
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
		dist = (pt - a).GetLength();
		return a;
	}

	// Voronoi related to bc
	float unom = DotProduct(pt - b, bc);
	float udenom = DotProduct(pt - c, b - c);

	if (sdenom <= 0.f && unom <= 0.f)
	{
		dist = (pt - b).GetLength();
		return b;
	}

	if (tdenom <= 0.f && udenom <= 0.f)
	{
		dist = (pt - c).GetLength();
		return c;
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
		Vector3 closest = a + dev;
		dist = (-toA - dev).GetLength();

		return closest;
	}

	// bc
	Vector3 toC = c - pt;
	float va = DotProduct(n, toB.Cross(toC));
	if (va <= 0.f && unom >= 0.f && udenom >= 0.f)
	{
		// closest feature is edge bc
		Vector3 dev = bc * (unom / (unom + udenom));
		Vector3 closest = b + dev;
		dist = (-toB - dev).GetLength();

		return closest;
	}

	// ac
	float vb = DotProduct(n, toC.Cross(toA));
	if (vb <= 0.f && tnom >= 0.f && tdenom >= 0.f)
	{
		// closest feature is edge ac
		Vector3 dev = ac * (tnom / (tnom + tdenom));
		Vector3 closest = a + dev;
		dist = (-toA - dev).GetLength();

		return closest;
	}

	// in this case pt project within the triangle 
	float u = va / (va + vb + vc);
	float v = vb / (va + vb + vc);
	float w = 1.f - u - v;
	Vector3 closest = a * u + b * v + c * w;
	dist = (pt - closest).GetLength();

	return closest;
}

Vector3 ProjectPointToTetraFeature(const Vector3& point, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, const Vector3& vert4, float& dist)
{
	float min_dist = INFINITY;
	Vector3 closest;

	// closest dist regarding all triangles
	Vector3 close = ProjectPointToPlaneFeature(point, vert1, vert2, vert3, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		closest = close;
	}

	close = ProjectPointToPlaneFeature(point, vert2, vert3, vert4, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		closest = close;
	}

	close = ProjectPointToPlaneFeature(point, vert1, vert2, vert4, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		closest = close;
	}

	close =ProjectPointToPlaneFeature(point, vert1, vert3, vert4, dist);
	if (dist < min_dist)
	{
		min_dist = dist;
		closest = close;
	}

	dist = min_dist;
	return closest;
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

Vector3 TransformBasisPoint(const Vector3& point, const Matrix44& basis)
{
	Vector4 pointV4 = Vector4(point.x, point.y, point.z, 1.f);

	Vector4 transformedPointV4 = basis * pointV4;

	Vector3 transformedPoint = Vector3(transformedPointV4.x, transformedPointV4.y, transformedPointV4.z);

	return transformedPoint;
}

/*
 * This assumes both interval comes from the same line, which means the 'the_other' - 'one_end'
 * is pointing in the same direction as the line they are generated from for both intervals
 */
float GetIntervalOverlapDirectional(const DirectionalInterval& interval1, const DirectionalInterval& interval2)
{
	Vector3 dir1 = interval1.m_max - interval1.m_min;
	Vector3 dir2 = interval2.m_max - interval2.m_min;
	ASSERT_OR_DIE(DotProduct(dir1, dir2) > 0.f, "Two intervals have different direction");

	if (interval1.IsPointSmaller(interval2.m_min, interval1.m_max) &&
		interval1.IsPointSmaller(interval1.m_min, interval2.m_min) && 
		interval1.IsPointSmaller(interval1.m_max, interval2.m_max))
		return ((interval1.m_max - interval2.m_min).GetLength());
	else if (interval1.IsPointSmaller(interval2.m_min, interval1.m_max) &&
			 interval1.IsPointSmaller(interval1.m_min, interval2.m_min) && 
			 interval1.IsPointSmaller(interval2.m_max, interval1.m_max))
		return interval2.GetIntervalLength();
	else if (interval1.IsPointSmaller(interval1.m_min, interval2.m_max) &&
			 interval1.IsPointSmaller(interval2.m_min, interval1.m_min) &&
			 interval1.IsPointSmaller(interval2.m_max, interval1.m_max))
		return ((interval2.m_max - interval1.m_min).GetLength());
	else if (interval1.IsPointSmaller(interval1.m_min, interval2.m_max) && 
			 interval1.IsPointSmaller(interval2.m_min, interval1.m_min) && 
			 interval1.IsPointSmaller(interval1.m_max, interval2.m_max))
		return interval1.GetIntervalLength();
	else 
		return 0.f;
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

std::set<Vector3> MinkowskiDifferenceHull(const std::set<Vector3>& hull_0, const std::set<Vector3>& hull_1)
{
	// A - B == A + (-B)
	std::set<Vector3> reversed_hull_1;
	for (const Vector3& from_1 : hull_1)
		reversed_hull_1.emplace(-from_1);

	return MinkowskiSumHull(hull_0, reversed_hull_1);
}

std::set<Vector3> MinkowskiSumHull(const std::set<Vector3>& hull_0, const std::set<Vector3>& hull_1)
{
	std::set<Vector3> sums;

	for (const Vector3& from_0 : hull_0)
	{
		for (const Vector3& from_1 : hull_1)
		{
			Vector3 sum = from_0 + from_1;
			sums.emplace(sum);
		}
	}

	return sums;
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

Matrix33 GetCanonicalTetrahedronCovariance()
{
	float a = 1.f / 60.f;
	float b = 1.f / 120.f;

	Vector3 i = Vector3(a, b, b);
	Vector3 j = Vector3(b, a, b);
	Vector3 k = Vector3(b, b, a);

	return Matrix33(i, j, k);
}

Matrix33 GetInertiaTensorFromCovariance(const Matrix33& cov)
{
	Matrix33 res = Matrix33::IDENTITY;
	res *= cov.GetTrace();

	res -= cov;

	return res;
}

Matrix33 TranslateCovariance(const Matrix33& cov, const Vector3& com, const float& mass, const Vector3& offset)
{
	float dot1 = DotProduct(offset, com);
	float dot2 = DotProduct(com, offset);
	float dot3 = DotProduct(offset, offset);

	return (cov + mass * (dot1 + dot2 + dot3));
}

bool AreFloatsCloseEnough(const float& f1, const float& f2)
{	
	return (abs(f1 - f2) < 0.01f);
}
