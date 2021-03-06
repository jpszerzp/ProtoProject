#pragma once

#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Vector4.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Core/Interval.hpp"

#include <set>

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
bool  CheckRandomChance( float chanceForSuccess );	
Vector3 GetRandomVector3(const Vector3& v1, const Vector3& v2);
Vector3 GetRandomVector3();

// physics
float GetDistance(const Vector2& a, const Vector2& b);
float GetDistanceSquared(const Vector2& a, const Vector2& b);
bool  DoAABBsOverlap( const AABB2& a, const AABB2& b );
float GetIntervalOverlapDirectional(const DirectionalInterval& interval1, const DirectionalInterval& interval2);

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
bool ProjectPlaneToSphere(Vector2 pos, float r, Vector3& out_pos);	// a spherical coord operation
Vector3 ProjectPointToPlane(const Vector3& point, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, float& dist);
Vector3 ProjectPointToPlaneFeature(const Vector3& point, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, float& dist);
Vector3 ProjectPointToTetraFeature(const Vector3& point, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3, const Vector3& vert4, float& dist);

bool  Quadratic(Vector2& out, float a, float b, float c);
int   ComputeManhattanDistanceInt(IntVector2 v1, IntVector2 v2);
float ComputeManhattanDistanceFloat(Vector2 v1, Vector2 v2);

// barycentric coord
float   ComputeTriangleArea(Vector3 v1, Vector3 v2, Vector3 v3);
Vector3 ComputeTriangleCenter(const Vector3& v1, const Vector3& v2, const Vector3& v3);

// lerp transform
Matrix44 LerpTransform(const Matrix44& m1, const Matrix44& m2, float fraction);

void SwapFloat(float& t1, float& t2);

// approximation
bool IsIdenticalFloat(float f1, float f2);
bool IsIdenticalVector3(Vector3 v1, Vector3 v2);

// minkowski
AABB2 MinkowskiAABBVsAABB(const AABB2& aabb1, const AABB2& aabb2);
std::set<Vector3> MinkowskiDifferenceHull(const std::set<Vector3>& hull_0, const std::set<Vector3>& hull_1);
std::set<Vector3> MinkowskiSumHull(const std::set<Vector3>& hull_0, const std::set<Vector3>& hull_1);

// collisions
bool IsPointInCircle(Vector2 pos, float r);
float DistPointToEdge(const Vector3& pt, const Vector3& vert1, const Vector3& vert2);
float DistPointToPlaneSigned(const Vector3& pt, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3);
float DistPointToPlaneUnsigned(const Vector3& pt, const Vector3& vert1, const Vector3& vert2, const Vector3& vert3);

Matrix33 GetCanonicalTetrahedronCovariance();
Matrix33 GetInertiaTensorFromCovariance(const Matrix33& cov);
Matrix33 TranslateCovariance(const Matrix33& cov, const Vector3& com, const float& mass, const Vector3& offset);

// approximation
bool AreFloatsCloseEnough(const float& f1, const float& f2);