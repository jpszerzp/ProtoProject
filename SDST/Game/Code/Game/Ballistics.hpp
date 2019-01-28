//#pragma once
//
//#include "Engine/Core/Primitive/Point.hpp"
//
//enum eBallisticsType
//{
//	PISTOL,
//	ARTILLERY,
//	FIREBALL,
//	LASER,
//	FREEFALL,
//	NUM_BALLISTICS_TYPE
//};
//
//class Ballistics : public Point
//{
//public:
//	eBallisticsType m_type = PISTOL;
//
//public:
//	Ballistics();
//	Ballistics(eBallisticsType type, Vector3 pos);
//	Ballistics(eBallisticsType type, Vector3 pos, bool frozen, Rgba color);
//	~Ballistics();
//
//	void ConfigureBallisticsType();
//
//	void Update(float deltaTime) override;
//	void UpdateInput(float deltaTime) override;
//};