#include "Engine/Physics/2D/QuadEntity.hpp"
#include "Engine/Physics/2D/PhysicsScene.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"

QuadEntity::QuadEntity(const AABB2& primitive, bool isConst)
{
	m_primitive = primitive;
	m_primitive.leftNormal = Vector2(-1.f, 0.f);
	m_primitive.upNormal = Vector2(0.f, 1.f);
	m_primitive.rightNormal = Vector2(1.f, 0.f);
	m_primitive.downNormal = Vector2(0.f, -1.f);

	m_const = isConst;

	m_velocity = Vector2::ZERO;

	Renderer* renderer = Renderer::GetInstance();
	m_colliderMesh = renderer->CreateOrGetMesh("quad_pcu_2d");

	m_orientation = 0.f;
	Vector2 min = primitive.mins;
	Vector2 max = primitive.maxs;
	Vector2 center = (min + max) * 0.5f;
	m_center = center;

	float xDim = primitive.GetDimensions().x;
	float yDim = primitive.GetDimensions().y;
	Vector3 pos = m_center.ToVector3(0.f);
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3(xDim, yDim, 1.f);
	m_quadTransform = Transform(pos, rot, scale);

	float boundDiscRadius = sqrtf(xDim * xDim + yDim * yDim) / 2.f;
	Vector3 boundDiscScale = Vector3(boundDiscRadius, boundDiscRadius, 1.f);
	m_boxBoundTransform = m_quadTransform;
	m_discBoundTransform = Transform(pos, rot, boundDiscScale);

	if (!m_const)
	{
		m_massData.m_mass = 1.f;
		//m_massData.m_mass = 100.f;
		m_massData.m_inertia = .01f;
		m_massData.m_invMass = 1.f / m_massData.m_mass;
		m_massData.m_invInertia = 1.f / m_massData.m_inertia;
	}
	else
	{
		m_massData.m_mass = 0.f;
		m_massData.m_inertia = 0.f;
	}

	if (m_massData.m_mass == 0.f)
	{
		m_massData.m_invMass = 0.f;
	}
	if (m_massData.m_inertia == 0.f)
	{
		m_massData.m_invInertia = 0.f;
	}

	m_boundCircle = Disc2(m_center, boundDiscRadius);
	m_discBoundMesh = Mesh::CreateDisc2D(VERT_PCU);

	m_boundAABB = m_primitive;
	m_boxBoundMesh = Mesh::CreateQuad2D(VERT_PCU);
}


QuadEntity::QuadEntity(const QuadEntity& copy)
{
	m_center = copy.m_center;
	m_quadTransform = copy.m_quadTransform;
	m_boxBoundTransform = copy.m_boxBoundTransform;
	m_discBoundTransform = copy.m_discBoundTransform;
	m_primitive = copy.m_primitive;
	m_boundCircle = copy.m_boundCircle;
	m_boundAABB = copy.m_boundAABB;
}

QuadEntity::~QuadEntity()
{

}


void QuadEntity::Update(float deltaTime)
{
	m_scene->ApplyGravity(this);

	UpdateInput();

	m_elasticityColor.r = (unsigned char)(255 * m_physicsMat.m_elasticity);
	m_frictionColor.b = (unsigned char)(255 * m_physicsMat.m_frictionCoef);

	if (!m_const && !m_kinematic)
	{
		Integrate(deltaTime);
		LimitSpeed(10000.f);
	}

	WrapAround();
}


void QuadEntity::UpdateInput()
{
	InputSystem* input = InputSystem::GetInstance();

	if (input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_0))
	{
		m_physicsMat.m_elasticity -= 0.01f;
		m_physicsMat.m_elasticity = ClampFloat(m_physicsMat.m_elasticity, 0.f, 1.f);
	}
	else if (input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_3))
	{
		m_physicsMat.m_elasticity += 0.01f;
		m_physicsMat.m_elasticity = ClampFloat(m_physicsMat.m_elasticity, 0.f, 1.f);
	}

	if (input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_6))
	{
		m_physicsMat.m_frictionCoef -= 0.01f;
		m_physicsMat.m_frictionCoef = ClampFloat(m_physicsMat.m_frictionCoef, 0.f, 1.f);
	}
	else if (input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_9))
	{
		m_physicsMat.m_frictionCoef += 0.01f;
		m_physicsMat.m_frictionCoef = ClampFloat(m_physicsMat.m_frictionCoef, 0.f, 1.f);
	}

	if (m_kinematic)
	{
		// receive user input for movement of kinematic shape
		if (input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
		{
			m_center.y += 2.f;
		}
		if (input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
		{
			m_center.y -= 2.f;
		}
		if (input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
		{
			m_center.x -= 2.f;
		}
		if (input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
		{
			m_center.x += 2.f;
		}
	}
}


void QuadEntity::UpdateBoundDisc()
{
	m_boundCircle.center = m_center;
}

void QuadEntity::UpdateBoundAABB()
{
	m_boundAABB = m_primitive;
}

void QuadEntity::UpdateTransformFinal()
{
	if (!m_const)
	{
		// update transform of non const
		Vector3 pos = m_center.ToVector3(0.f);
		m_quadTransform.SetLocalPosition(pos);
		m_boxBoundTransform.SetLocalPosition(pos);
		m_discBoundTransform.SetLocalPosition(pos);

		// update box
		float boxWidth = m_primitive.GetDimensions().x;
		float boxHeight = m_primitive.GetDimensions().y;
		Vector2 boxMin = m_center - Vector2(boxWidth / 2.f, boxHeight / 2.f);
		Vector2 boxMax = m_center + Vector2(boxWidth / 2.f, boxHeight / 2.f);
		m_primitive = AABB2(boxMin, boxMax);
		m_primitive.leftNormal = Vector2(-1.f, 0.f);
		m_primitive.upNormal = Vector2(0.f, 1.f);
		m_primitive.rightNormal = Vector2(1.f, 0.f);
		m_primitive.downNormal = Vector2(0.f, -1.f);

		UpdateBoundDisc();
		UpdateBoundAABB();
	}
}

void QuadEntity::Integrate(float deltaTime)
{
	if (!m_continued)
	{
		m_center = m_center + m_velocity * deltaTime;
	}
	else
	{
		m_continued = false;
	}
	m_acceleration = m_netForce * (m_massData.m_invMass);
	m_velocity = m_velocity + m_acceleration * deltaTime;

	/*
	if (m_drag)
	{
		m_velocity *= pow(m_damping, deltaTime);				// impose drag (in 3D)
	}
	*/

	m_orientation = m_orientation + m_signedRotationVel * deltaTime;
	m_signedRotationVel = m_signedRotationVel + m_signedRotationAcc * deltaTime;
}

void QuadEntity::WrapAround()
{
	Window* window = Window::GetInstance();
	float windowWidth = window->GetWindowWidth();
	float windowHeight = window->GetWindowHeight();
	float leftBound = -windowWidth / 2.f;
	float rightBound = windowWidth / 2.f;
	float upBound = windowHeight / 2.f;
	float bottomBound = -windowHeight / 2.f;

	Vector2 center = (m_primitive.mins + m_primitive.maxs) / 2.f;
	float xHalfDim = m_primitive.GetDimensions().x / 2.f;
	float yHalfDim = m_primitive.GetDimensions().y / 2.f;

	if (center.x > ( rightBound + xHalfDim ))
	{
		// entity wraps to left side of screen
		m_center.x = leftBound - xHalfDim;
	}

	else if (center.x < (leftBound - xHalfDim))
	{
		// entity wraps to right side of screen
		m_center.x = rightBound + xHalfDim;
	}

	if (center.y > ( upBound + yHalfDim ))
	{
		// wraps to down side of screen
		m_center.y = bottomBound - yHalfDim;
	}

	else if (center.y < bottomBound - yHalfDim)
	{
		// wraps to top side
		m_center.y = upBound + yHalfDim;
	}
}

void QuadEntity::Render(Renderer* renderer)
{
	// quad collider
	Shader* shader = renderer->CreateOrGetShader("2d_default");
	renderer->UseShader(shader);

	Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());

	renderer->m_objectData.model = m_quadTransform.GetWorldMatrix();

	Rgba color;
	if (m_kinematic)
	{
		color = Rgba::CYAN;
	}
	else
	{
		if (m_const)
		{
			color = m_constColor;
		}
		else
		{
			color = m_nonConstColor;
		}
		color.r = (unsigned char)((float)(color.r + m_elasticityColor.r + m_frictionColor.r) / 3.f);
		color.g = (unsigned char)((float)(color.g + m_elasticityColor.g + m_frictionColor.g) / 3.f);
		color.b = (unsigned char)((float)(color.b + m_elasticityColor.b + m_frictionColor.b) / 3.f);
		color.a = (unsigned char)((float)(color.a + m_elasticityColor.a + m_frictionColor.a) / 3.f);
	}

	Vector4 colorV4;
	color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);
	renderer->m_colorData.rgba = colorV4;
	renderer->SetColorUBO(shader->GetShaderProgram()->GetHandle());

	renderer->DrawMesh(m_colliderMesh);

	// bound aabb
	if (m_drawBoundAABB)
	{
		shader = renderer->CreateOrGetShader("2d_default_wireframe");
		renderer->UseShader(shader);

		texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->m_objectData.model = m_boxBoundTransform.GetWorldMatrix();

		color = Rgba::WHITE;
		color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);
		renderer->m_colorData.rgba = colorV4;
		renderer->SetColorUBO(shader->GetShaderProgram()->GetHandle());

		renderer->DrawMesh(m_boxBoundMesh);
	}

	if (m_drawBoundDisc)
	{
		shader = renderer->CreateOrGetShader("2d_default_wireframe");
		renderer->UseShader(shader);

		texture = renderer->CreateOrGetTexture("Data/Images/white.png");
		renderer->SetTexture2D(0, texture);
		renderer->SetSampler2D(0, texture->GetSampler());

		renderer->m_objectData.model = m_discBoundTransform.GetWorldMatrix();

		color = Rgba::BLUE;
		color.GetAsFloats(colorV4.x, colorV4.y, colorV4.z, colorV4.w);
		renderer->m_colorData.rgba = colorV4;
		renderer->SetColorUBO(shader->GetShaderProgram()->GetHandle());

		renderer->DrawMesh(m_discBoundMesh);
	}
}