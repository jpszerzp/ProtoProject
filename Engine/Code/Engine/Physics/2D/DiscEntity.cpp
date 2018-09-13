#include "Engine/Physics/2D/DiscEntity.hpp"
#include "Engine/Physics/2D/PhysicsScene.hpp"
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"

DiscEntity::DiscEntity(const DiscEntity& copy)
{
	// only needs these to generate a disc for collision test
	m_center = copy.m_center;
	m_discTransform = copy.m_discTransform;
	m_discBoundTransform = copy.m_discBoundTransform;
	m_boxBoundTransform = copy.m_boxBoundTransform;
	m_primitive = copy.m_primitive;
	m_boundCircle = copy.m_boundCircle;
	m_boundAABB = copy.m_boundAABB;
}

DiscEntity::DiscEntity(const Disc2& primitive, bool isConst)
{
	m_primitive = primitive;
	m_const = isConst;

	m_velocity = Vector2::ZERO;

	Renderer* renderer = Renderer::GetInstance();
	m_colliderMesh = renderer->CreateOrGetMesh("disc_pcu_2d");

	m_orientation = 0.f;
	m_center = m_primitive.center;

	float radius = m_primitive.radius;
	Vector3 pos = m_center.ToVector3(0.f);
	Vector3 rot = Vector3::ZERO;
	Vector3 scale = Vector3(radius, radius, 1.f);
	m_discTransform = Transform(pos, rot, scale);

	Vector3 discScale = Vector3(1.2f * radius, 1.2f * radius, 1.f);
	Vector3 boxScale = Vector3(2.f * radius, 2.f * radius, 1.f);
	m_boxBoundTransform = Transform(pos, rot, boxScale);
	m_discBoundTransform = Transform(pos, rot, discScale);

	if (!m_const)
	{
		m_massData.m_mass = 1.f;
		m_massData.m_inertia = 100.f;
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

	m_boundCircle = Disc2(m_center, 1.2f * radius);
	m_discBoundMesh = Mesh::CreateDisc2D(VERT_PCU);

	// bound box
	Vector2 halfDim = Vector2(m_primitive.radius, m_primitive.radius);
	Vector2 boundMin = m_center - halfDim;
	Vector2 boundMax = m_center + halfDim;
	m_boundAABB = AABB2(boundMin, boundMax);
	m_boxBoundMesh = Mesh::CreateQuad2D(VERT_PCU);
}

DiscEntity::~DiscEntity()
{
	delete m_orientMesh;
	m_orientMesh = nullptr;
}


void DiscEntity::Update(float deltaTime)
{
	m_passedBroadphase = false;

	if (m_orientMesh != nullptr)
	{
		delete m_orientMesh;
		m_orientMesh = nullptr;
	}
	float radius = m_primitive.radius;
	Vector2 endDeviation =  Vector2(CosDegrees(m_orientation) * radius, SinDegrees(m_orientation) * radius);
	Vector2 end = m_center + endDeviation;
	Rgba lineTint = Rgba::WHITE;
	m_orientMesh = Mesh::CreateLineImmediate2D(m_center, end, lineTint, VERT_PCU);

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

void DiscEntity::UpdateInput()
{
	InputSystem* input = InputSystem::GetInstance();

	if (input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
	{
		m_physicsMat.m_elasticity -= 0.01f;
		m_physicsMat.m_elasticity = ClampFloat(m_physicsMat.m_elasticity, 0.f, 1.f);
	}
	 if (input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5))
	{
		m_physicsMat.m_elasticity += 0.01f;
		m_physicsMat.m_elasticity = ClampFloat(m_physicsMat.m_elasticity, 0.f, 1.f);
	}
	if (input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
	{
		m_physicsMat.m_frictionCoef -= 0.01f;
		m_physicsMat.m_frictionCoef = ClampFloat(m_physicsMat.m_frictionCoef, 0.f, 1.f);
	}
	if (input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
	{
		m_physicsMat.m_frictionCoef += 0.01f;
		m_physicsMat.m_frictionCoef = ClampFloat(m_physicsMat.m_frictionCoef, 0.f, 1.f);
	}
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_F3))
	{
		m_drawBoundAABB = !m_drawBoundAABB;
	}
	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_F4))
	{
		m_drawBoundDisc = !m_drawBoundDisc;
	}
}

void DiscEntity::UpdateBoundAABB()
{
	Vector2 halfDim = Vector2(m_primitive.radius, m_primitive.radius);
	Vector2 boundMin = m_center - halfDim;
	Vector2 boundMax = m_center + halfDim;
	m_boundAABB = AABB2(boundMin, boundMax);
}

void DiscEntity::UpdateBoundDisc()
{
	m_boundCircle.center = m_center;
}

void DiscEntity::UpdateTransformFinal()
{
	if (!m_const)
	{
		// update entity transform
		Vector3 pos = m_center.ToVector3(0.f);
		Vector3 euler = Vector3(0.f, 0.f, m_orientation);
		m_discTransform.SetLocalPosition(pos);
		m_discTransform.SetLocalRotation(euler);

		// update bound transforms
		Vector3 boundEuler = Vector3::ZERO;
		Vector3 discBoundScale = Vector3(m_primitive.radius * 1.2f, m_primitive.radius * 1.2f, 1.f);
		Vector3 boxBoundScale = Vector3(m_primitive.radius * 2.f, m_primitive.radius * 2.f, 1.f);
		m_discBoundTransform = Transform(pos, boundEuler, discBoundScale);
		m_boxBoundTransform = Transform(pos, boundEuler, boxBoundScale);

		// update primitive
		m_primitive = Disc2(m_center, m_primitive.radius);

		// update circle bound
		UpdateBoundDisc();

		// update aabb bound
		UpdateBoundAABB();
	}
}

void DiscEntity::WrapAround()
{
	Window* window = Window::GetInstance();
	float windowWidth = window->GetWindowWidth();
	float windowHeight = window->GetWindowHeight();
	float leftBound = -windowWidth / 2.f;
	float rightBound = windowWidth / 2.f;
	float upBound = windowHeight / 2.f;
	float bottomBound = -windowHeight / 2.f;

	Disc2 disc = m_primitive;
	Vector2 center = disc.center;
	float radius = disc.radius;

	if (center.x > ( rightBound + radius ))
	{
		// entity wraps to left side of screen
		m_center.x = leftBound - radius;
	}

	else if (center.x < (leftBound - radius))
	{
		// entity wraps to right side of screen
		m_center.x = rightBound + radius;
	}

	if (center.y > ( upBound + radius ))
	{
		// wraps to down side of screen
		m_center.y = bottomBound - radius;
	}

	else if (center.y < bottomBound - radius)
	{
		// wraps to top side
		m_center.y = upBound + radius;
	}
}

void DiscEntity::Render(Renderer* renderer)
{
	Shader* shader = renderer->CreateOrGetShader("2d_direct_opague");
	renderer->UseShader(shader);

	Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());

	renderer->m_objectData.model = Matrix44::IDENTITY;

	renderer->DrawMesh(m_orientMesh);

	// disc collider
	shader = renderer->CreateOrGetShader("2d_default");
	renderer->UseShader(shader);

	texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());

	renderer->m_objectData.model = m_discTransform.GetWorldMatrix();

	Rgba color;
	if (m_passedBroadphase && m_scene->m_drawBP)
	{
		if (m_scene->m_bpScheme == BP_BOUND)
		{
			color = Rgba::GREEN;
		}
		else if (m_scene->m_bpScheme == BP_SAP)
		{
			color = Rgba::CYAN;
		}
		else if (m_scene->m_bpScheme == BP_QUAD)
		{
			color = Rgba::MEGENTA;
		}
	}
	else
	{
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

bool DiscEntity::CollideWithDiscEntity(DiscEntity& discEntity)
{
	Disc2 myDisc = GenerateDisc2();
	Disc2 otherDisc = discEntity.GenerateDisc2();

	bool collided = DoDiscsOverlap(myDisc, otherDisc);
	return collided;
}

bool DiscEntity::CollideWithDiscEntity(DiscEntity& discEntity, Vector2& overlap)
{
	Disc2 myDisc = GenerateDisc2();
	Disc2 otherDisc = discEntity.GenerateDisc2();

	bool collided = DoDiscsOverlap(myDisc, otherDisc);

	if (collided)
	{
		Vector2 displacement = otherDisc.center - myDisc.center;
		Vector2 overlapDirectionNormalized = displacement.GetNormalized();
		float dist = displacement.GetLength();
		float overlapAmount = myDisc.radius + otherDisc.radius - dist;

		overlap = overlapDirectionNormalized * overlapAmount;
	}
	else
	{
		overlap = Vector2::ZERO;
	}

	return collided;
}


Disc2 DiscEntity::GenerateDisc2()
{
	return m_primitive;
}

float DiscEntity::GetDisc2Radius()
{
	return m_primitive.radius;
}

Vector2 DiscEntity::GetDisc2Center()
{
	return m_primitive.center;
}
