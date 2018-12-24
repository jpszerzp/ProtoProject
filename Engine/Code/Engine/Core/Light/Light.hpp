#pragma once

#include "Engine/Core/GameObject.hpp"
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"

enum eLightType
{
	DIRECTIONAL_LIGHT,
	SPOT_LIGHT,
	POINT_LIGHT,
	NUM_LIGHT_TYPE
};

class Light : public GameObject
{
public:
	Rgba		m_lightColor;
	Vector3		m_attenuation;
	Vector3		m_specAttenuation;
	Vector3		m_direction;
	float		m_directionFac;
	float		m_innerAngle;
	float		m_outerAngle;
	bool		m_shadowCast = false;
	Matrix44	m_shadowVP;

	Vector3 m_mat_amb;
	Vector3 m_mat_diff;
	Vector3 m_mat_spec;
	
private:
	// atten
	float m_atten_const;
	float m_atten_linear;
	float m_atten_quad;

public:
	Light(Vector3 pos, Vector3 rot, Vector3 scale, Rgba color,
		Vector3 attenuation, Vector3 specAttenuation, Vector3 direction,
		float innerAngle, float outerAngle, float directionFac);
	~Light();

	void Update(float deltaTime) = 0;
	void Render(Renderer* renderer) = 0;

	Rgba GetColor() const { return m_lightColor; }
	float GetAttenConst() const { return m_atten_const; }
	float GetAttenLinear() const { return m_atten_linear; }
	float GetAttenQuadratic() const { return m_atten_quad; }
	Vector3 GetDirection() const { return m_direction;}

	void SetAttenConst(float val) { m_atten_const = val;}
	void SetAttenLinear(float val) { m_atten_linear = val; }
	void SetAttenQuad(float val) { m_atten_quad=val;}
};