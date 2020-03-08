#pragma once

#include "Engine/Math/Vector2.hpp"
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/FrameBuffer.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Core/Transform.hpp"
#include "Engine/Core/Ray3.hpp"

class Camera
{
public:
	Camera(Vector3 pos = Vector3::ZERO);
	~Camera();

	// will be implemented later
	void SetColorTarget(Texture* color_target) { m_output.SetColorTarget(color_target); }
	void SetDepthStencilTarget(Texture* depth_target) { m_output.SetDepthStencilTarget(depth_target); }

	// projection settings
	void SetView(Matrix44 view) { m_view = view; }
	void SetProjectionOrtho(float sizeX, float sizeY, float theNear = 0.f, float theFar = 1.f); 
	void SetProjectionOrtho(Vector2 bl, Vector2 tr, float theNear = 0.f, float theFar = 1.f);
	void SetProjectionPerspective(float fovDegrees, float aspect, float nz, float fz);
	void SetProjectionPerspective(float t, float b, float l, float r, float nz, float fz);
	void SetAspect(float value) { m_aspect = value; }
	void SetFOV(float value) { m_fov = value; }

	float			GetAspect() const { return m_aspect; }
	float			GetFOV() const { return m_fov; }
	Matrix44		GetProjection() const { return m_proj; }
	Matrix44		GetView() const { return m_view; }
	GLuint			GetFrameBufferHandle() const { return m_output.GetHandle(); }
	FrameBuffer&	GetFrameBuffer() { return m_output; }
	Transform&		GetTransform() { return m_transform; }
	Ray3			GetRay() const { return m_forwardRay; }
	Ray3			ComputeRay() const;

	Vector3 GetLocalForward() const;
	Vector3 GetLocalUp() const;
	Vector3 GetLocalRight() const;

	Vector3 GetWorldForward() const;
	Vector3 GetWorldUp() const;
	Vector3 GetWorldRight() const;
	Vector3 GetWorldPosition() const;

	void Finalize();

	Matrix44 LookAtLocal(Vector3 position, Vector3 target, Vector3 up = Vector3::UP);

public:
	float	  m_fov;
	float	  m_aspect;
	Transform m_transform;		// model
	Matrix44  m_view;			// inverse of model/view
	Matrix44  m_proj;			// projection

	FrameBuffer m_output;

	std::vector<Material*> m_effectMats;

	Ray3 m_forwardRay;
};