#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Disc2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Sphere3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Plane.hpp"

#define PI 3.14159265359f
#define SLERP_ANGLE_THRESHOLD 0.05f

// radians and degree 
float ConvertRadiansToDegrees(float radians);
float ConvertDegreesToRadians(float degrees);
float CosDegrees(float degrees);
float SinDegrees(float degrees);

// random num (int/float)
float GetRandomFloatInRange(float min, float max);
float GetRandomFloatZeroToOne();
int	  GetRandomIntInRange(int minInclusive, int maxInclusive);
int	  GetRandomIntLessThan(int maxNotInclusive);
Vector3 GetRandomVector3(const Vector3& v1, const Vector3& v2);
bool  CheckRandomChance( float chanceForSuccess );	

// physics
float GetDistance(const Vector2& a, const Vector2& b);
float GetDistanceSquared(const Vector2& a, const Vector2& b);
bool  DoDiscsOverlap (const Disc2& a, const Disc2& b);
bool  DoDiscsOverlap (const Vector2& aCenter, float aRadius, const Vector2& bCenter, float bRadius);
bool  DoAABBsOverlap( const AABB2& a, const AABB2& b );
float GetSquredDistPointVsAABB3(const Vector3& point, const AABB3& aabb3);
bool  SphereVsAABB3Intersection(const Sphere3& sphere, const AABB3& aabb3);
bool  SphereVsOBB3Intersection(const Sphere3& sphere, const OBB3& obb3);
bool  SphereVsSphereIntersection(const Sphere3& s1, const Sphere3& s2);
bool  AABB3VsPlaneIntersection(const AABB3& aabb, const Plane& plane);
bool  AABB3VsAABB3Intersection(const AABB3& aabb_1, const AABB3& aabb_2);
bool  AABB3VsAABB3Intersection(const AABB3& aabb_1, const AABB3& aabb_2,
	Vector3& axis, float& overlap);

// rounding, clamping and range mapping
float RangeMapFloat(float inValue, float inStart, float inEnd, float outStart, float outEnd);
float ClampZeroToOne(float value);
float ClampZeroToFloat(float valueClampTo, float valueClamped);
float ClampFloatNegativeOneToOne( float inValue );
int   ClampInt( int inValue, int min, int max );
float ClampFloat( float inValue, float min, float max );
float GetFractionInRange( float inValue, float rangeStart, float rangeEnd );
int   RoundToNearestInt( float inValue );	
float RoundToNearest05(float inValue);

// angle/vectors
float GetAngularDisplacement( float startDegrees, float endDegrees );
float TurnToward( float currentDegrees, float goalDegrees, float maxTurnDegrees );
float DotProduct( const Vector2& a, const Vector2& b );
float DotProduct( const Vector3& a, const Vector3& b );
float DotProduct( const Vector4& a, const Vector4& b );

// bitflag utilities
bool AreBitsSet( unsigned char bitFlags8, unsigned char flagsToCheck );
bool AreBitsSet( unsigned int bitFlags32, unsigned int flagsToCheck );
void SetBits( unsigned char& bitFlags8, unsigned char flagsToSet );
void SetBits( unsigned int& bitFlags32, unsigned int flagsToSet );
void ClearBits( unsigned char& bitFlags8, unsigned char flagToClear );
void ClearBits( unsigned int& bitFlags32, unsigned int flagToClear );
int  CheckBit(int& subject, int bitPos);
void RightShiftLostBit(std::vector<uint8_t>& bits, size_t* num, int shiftNum);
void RightShiftRemainedBit(std::vector<uint8_t>& remained, size_t* num, int shiftNum);
uint8_t	 RightShiftLostBitUnit(size_t* num);
void SetHighBit(std::vector<uint8_t>& subject, uint8_t zero_one);
size_t DecimalFromBinary(std::vector<uint8_t>& binary);
std::vector<uint8_t> BinaryFromDecimal(size_t decimal);
std::vector<uint8_t> BinaryFromDecimal16(size_t decimal);
uint16_t BufferSizeHardCode16(std::string str);

// easing function
float	SmoothStart2( float t ); // 2nd-degree smooth start (a.k.a. �quadratic ease in�)
float	SmoothStart3( float t ); // 3rd-degree smooth start (a.k.a. �cubic ease in�)
float	SmoothStart4( float t ); // 4th-degree smooth start (a.k.a. �quartic ease in�)
float	SmoothStop2( float t ); // 2nd-degree smooth start (a.k.a. �quadratic ease out�)
float	SmoothStop3( float t ); // 3rd-degree smooth start (a.k.a. �cubic ease out�)
float	SmoothStop4( float t ); // 4th-degree smooth start (a.k.a. �quartic ease out�)
float	SmoothStep3( float t ); // 3rd-degree smooth start/stop (a.k.a. �smoothstep�)

// interpolation
int			  Interpolate( int start, int end, float fractionTowardEnd );
float		  Interpolate( float start, float end, float fractionTowardEnd );
unsigned char Interpolate( unsigned char start, unsigned char end, float fractionTowardEnd );
Vector3		  Interpolate(Vector3 start, Vector3 end, float fractionTowardEnd);
Vector3		  Slerp(const Vector3& v1, const Vector3& v2, float fraction);
Vector3		  SlerpUnit(const Vector3& v1, const Vector3& v2, float fraction);

// other set from text
std::vector<int> SetFromText(const char* text);

// coordinate transformation
Vector2 PolarToCartesian(float radius, float degree);
Vector3 PolarToCartesian( float radius, float rotationDeg, float azimuthDeg ); 
Vector3 PolarToCartesian( Vector3 spherical ); 
Vector3 CartesianToPolar( Vector3 position );
void MakeOrthonormalBasis(const Vector3& x, Vector3& y, Vector3& z);
void MakeOrthonormalBasisOpt(const Vector3& x, Vector3& y, Vector3& z);
//void MakeOrthonormalBasisStable(const Vector3& x);
bool ProjectPlaneToSphere(Vector2 pos, float r, Vector3& out_pos);	// a spherical coord operation

bool  Quadratic(Vector2& out, float a, float b, float c);
int   ComputeManhattanDistanceInt(IntVector2 v1, IntVector2 v2);
float ComputeManhattanDistanceFloat(Vector2 v1, Vector2 v2);

// barycentric coord
float   ComputeTriangleArea(Vector3 v1, Vector3 v2, Vector3 v3);
Vector3 ComputeTriangleCenter(Vector3 v1, Vector3 v2, Vector3 v3);

// lerp transform
Matrix44 LerpTransform(const Matrix44& m1, const Matrix44& m2, float fraction);

void SwapFloat(float& t1, float& t2);

// approximation
bool IsIdenticalFloat(float f1, float f2);
bool IsIdenticalVector3(Vector3 v1, Vector3 v2);

// minkowski
AABB2 MinkowskiAABBVsAABB(const AABB2& aabb1, const AABB2& aabb2);

// more shape test
bool IsPointInCircle(Vector2 pos, float r);