#include "Engine/Math/OBB3.hpp"

const Vector3& OBB3::GetFTL() const
{
	return GetCenter() - GetHalfExtX() + GetHalfExtY() - GetHalfExtZ();
}

const Vector3& OBB3::GetFBL() const
{
	return GetCenter() - GetHalfExtX() - GetHalfExtY() - GetHalfExtZ();
}

const Vector3& OBB3::GetFBR() const
{
	return GetCenter() + GetHalfExtX() - GetHalfExtY() - GetHalfExtZ();
}

const Vector3& OBB3::GetFTR() const
{
	return GetCenter() + GetHalfExtX() + GetHalfExtY() - GetHalfExtZ();
}

const Vector3& OBB3::GetBTL() const
{
	return GetCenter() - GetHalfExtX() + GetHalfExtY() + GetHalfExtZ();
}

const Vector3& OBB3::GetBBL() const
{
	return GetCenter() - GetHalfExtX() - GetHalfExtY() + GetHalfExtZ();
}

const Vector3& OBB3::GetBBR() const
{
	return GetCenter() + GetHalfExtX() - GetHalfExtY() + GetHalfExtZ();
}

const Vector3& OBB3::GetBTR() const
{
	return GetCenter() + GetHalfExtX() + GetHalfExtY() + GetHalfExtZ();
}

