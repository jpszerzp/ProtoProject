#include "Engine/Physics/2D/RectangleEntity.hpp"
#include "Engine/Physics/2d/PhysicsScene.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Mesh.hpp"

RectangleEntity::RectangleEntity(const OBB2& primitive, bool isConst)
{
	m_primitive = primitive;
	m_const = isConst;

	m_velocity = Vector2::ZERO;

	Renderer* renderer = Renderer::GetInstance();
	m_colliderMesh = renderer->CreateOrGetMesh("quad_pcu_2d");

	m_orientation = primitive.m_rotation;
	m_center = primitive.m_center;
	Vector2 scale = primitive.m_scale;

	Vector3 pos = m_center.ToVector3(0.f);
	Vector3 rot = Vector3(0.f, 0.f, m_orientation);
	Vector3 scale3 = scale.ToVector3(1.f);
	m_rectangleTransform = Transform(pos, rot, scale3);

	UpdateBoundAABB();
	float boundBoxScaleX = m_boundAABB.maxs.x - m_boundAABB.mins.x;
	float boundBoxScaleY = m_boundAABB.maxs.y - m_boundAABB.mins.y;
	Vector3 boundBoxScale = Vector3(boundBoxScaleX, boundBoxScaleY, 1.f);
	float boundDiscRadius = sqrtf(scale.x * scale.x + scale.y * scale.y) / 2.f;
	Vector3 boundDiscScale = Vector3(boundDiscRadius, boundDiscRadius, 1.f);
	Vector3 boundBoxAndDiscRot = Vector3::ZERO;
	m_boxBoundTransform = Transform(pos, boundBoxAndDiscRot, boundBoxScale);
	m_discBoundTransform = Transform(pos, boundBoxAndDiscRot, boundDiscScale);

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

	m_boxBoundMesh = Mesh::CreateQuad2D(VERT_PCU);
}


RectangleEntity::RectangleEntity(const RectangleEntity& copy)
{
	m_center = copy.m_center;
	m_rectangleTransform = copy.m_rectangleTransform;
	m_boxBoundTransform = copy.m_boxBoundTransform;
	m_discBoundTransform = copy.m_discBoundTransform;
	m_primitive = copy.m_primitive;
	m_boundCircle = copy.m_boundCircle;
	m_boundAABB = copy.m_boundAABB;
}


RectangleEntity::~RectangleEntity()
{

}

void RectangleEntity::Update(float deltaTime)
{
	m_passedBroadphase = false;

	m_scene->ApplyGravity(this);

	UpdateInput();

	m_elasticityColor.r = (unsigned char)(255 * m_physicsMat.m_elasticity);
	m_frictionColor.b = (unsigned char)(255 * m_physicsMat.m_frictionCoef);

	if (!m_const)
	{
		Integrate(deltaTime);
		LimitSpeed(10000.f);
	}

	WrapAround();
}

void RectangleEntity::UpdateInput()
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

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_F3))
	{
		m_drawBoundAABB = !m_drawBoundAABB;
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_F4))
	{
		m_drawBoundDisc = !m_drawBoundDisc;
	}
}


void RectangleEntity::UpdateBoundAABB()
{
	Vector2 bl = m_primitive.GenerateBL();
	Vector2 br = m_primitive.GenerateBR();
	Vector2 tl = m_primitive.GenerateTL();
	Vector2 tr = m_primitive.GenerateTR();

	float minX = INFINITY;
	float minY = INFINITY;
	float maxX = -INFINITY;
	float maxY = -INFINITY;
	if (bl.x < minX)
	{
		minX = bl.x;
	}
	if (br.x < minX)
	{
		minX = br.x;
	}
	if (tl.x < minX)
	{
		minX = tl.x;
	}
	if (tr.x < minX)
	{
		minX = tr.x;
	}

	if (bl.y < minY)
	{
		minY = bl.y;
	}
	if (br.y < minY)
	{
		minY = br.y;
	}
	if (tl.y < minY)
	{
		minY = tl.y;
	}
	if (tr.y < minY)
	{
		minY = tr.y;
	}

	if (bl.x > maxX)
	{
		maxX = bl.x;
	}
	if (br.x > maxX)
	{
		maxX = br.x;
	}
	if (tl.x > maxX)
	{
		maxX = tl.x;
	}
	if (tr.x > maxX)
	{
		maxX = tr.x;
	}

	if (bl.y > maxY)
	{
		maxY = bl.y;
	}
	if (br.y > maxY)
	{
		maxY = br.y;
	}
	if (tl.y > maxY)
	{
		maxY = tl.y;
	}
	if (tr.y > maxY)
	{
		maxY = tr.y;
	}

	Vector2 boundMin = Vector2(minX, minY);
	Vector2 boundMax = Vector2(maxX, maxY);
	m_boundAABB = AABB2(boundMin, boundMax);
}

void RectangleEntity::UpdateBoundDisc()
{
	m_boundCircle.center = m_center;
}

void RectangleEntity::UpdateTransformFinal()
{
	if (!m_const)
	{
		// update transform of non const
		Vector3 pos = m_center.ToVector3(0.f);
		Vector3 euler = Vector3(0.f, 0.f, m_orientation);
		m_rectangleTransform.SetLocalPosition(pos);
		m_rectangleTransform.SetLocalRotation(euler);

		// update bound disc
		UpdateBoundDisc();

		// update bound aabb
		UpdateBoundAABB();

		Vector3 scale = m_rectangleTransform.GetLocalScale();
		Vector3 boundEuler = Vector3::ZERO;
		float boundBoxScaleX = m_boundAABB.maxs.x - m_boundAABB.mins.x;
		float boundBoxScaleY = m_boundAABB.maxs.y - m_boundAABB.mins.y;
		Vector3 boundBoxScale = Vector3(boundBoxScaleX, boundBoxScaleY, 1.f);
		float boundDiscRadius = sqrtf(scale.x * scale.x + scale.y * scale.y) / 2.f;
		Vector3 boundDiscScale = Vector3(boundDiscRadius, boundDiscRadius, 1.f);
		m_discBoundTransform = Transform(pos, boundEuler, boundDiscScale);
		m_boxBoundTransform = Transform(pos, boundEuler, boundBoxScale);

		// update primitive
		Vector2 scale2 = Vector2(scale.x, scale.y);
		m_primitive = OBB2(m_center, euler.z, scale2);
	}
}

void RectangleEntity::WrapAround()
{
	Window* window = Window::GetInstance();
	float windowWidth = window->GetWindowWidth();
	float windowHeight = window->GetWindowHeight();
	float leftBound = -windowWidth / 2.f;
	float rightBound = windowWidth / 2.f;
	float upBound = windowHeight / 2.f;
	float bottomBound = -windowHeight / 2.f;

	// bound circle is used for wrap-around of rectangles
	Vector2 boundCenter = m_boundCircle.center;
	float boundRadius = m_boundCircle.radius;

	if (boundCenter.x > ( rightBound + boundRadius ))
	{
		// entity wraps to left side of screen
		m_center.x = leftBound - boundRadius;
	}

	else if (boundCenter.x < (leftBound - boundRadius))
	{
		// entity wraps to right side of screen
		m_center.x = rightBound + boundRadius;
	}

	if (boundCenter.y > ( upBound + boundRadius ))
	{
		// wraps to down side of screen
		m_center.y = bottomBound - boundRadius;
	}

	else if (boundCenter.y < bottomBound - boundRadius)
	{
		// wraps to top side
		m_center.y = upBound + boundRadius;
	}
}


void RectangleEntity::Render(Renderer* renderer)
{
	// quad collider
	Shader* shader = renderer->CreateOrGetShader("2d_default");
	renderer->UseShader(shader);

	Texture* texture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, texture);
	renderer->SetSampler2D(0, texture->GetSampler());

	renderer->m_objectData.model = m_rectangleTransform.GetWorldMatrix();

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
		shader = renderer->CreateOrGetShader("2d_direct_wireframe");
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