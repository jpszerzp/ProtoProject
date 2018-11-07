#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/Math/Matrix33.hpp"
#include "Engine/Math/Primitive3.hpp"
#include "Engine/Core/Transform.hpp"

class Entity3;

class OBB3
{
private:
	Entity3* m_entity = nullptr;

public:
	Vector3 m_center;			// OBB center point
	Vector3 m_forward;
	Vector3 m_up;
	Vector3 m_right;
	Vector3 m_halfExt;			// halfway extends of OBB along basis 

public:
	OBB3(){}
	OBB3(Vector3 center, Vector3 forward, Vector3 up, Vector3 right, Vector3 halfExt)
		: m_center(center), m_forward(forward), m_up(up), m_right(right), m_halfExt(halfExt){}
	~OBB3(){}

	float GetDiagonalHalf() const { return m_halfExt.GetLength(); }
	float GetFullExtX() const { return m_halfExt.x * 2.f; }
	float GetFullExtY() const { return m_halfExt.y * 2.f; }
	float GetFullExtZ() const { return m_halfExt.z * 2.f; }
	Vector3 GetHalfExt() const { return m_halfExt; }
	Vector3 GetHalfExtCopy() const { return m_halfExt; }
	Vector3 GetHalfExtX() const { return m_right * m_halfExt.x; }
	Vector3 GetHalfExtY() const { return m_up * m_halfExt.y; }
	Vector3 GetHalfExtZ() const { return m_forward * m_halfExt.z; }
	Vector3 GetCenter() const { return m_center; }
	Vector3 GetFTL() const;
	Vector3 GetFBL() const;
	Vector3 GetFBR() const;
	Vector3 GetFTR() const;
	Vector3 GetBTL() const;
	Vector3 GetBBL() const;
	Vector3 GetBBR() const;
	Vector3 GetBTR() const;
	Entity3* GetEntity() const { return m_entity; }

	void SetCenter(const Vector3& center);
	void SetEntity(Entity3* ent) { m_entity = ent; }
	void SetForward(const Vector3& forward) { m_forward = forward; }
	void SetUp(const Vector3& up) { m_up = up; }
	void SetRight(const Vector3& right) { m_right = right; }
};