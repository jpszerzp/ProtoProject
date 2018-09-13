#include "Game/ProtoState.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Console/DevConsole.hpp"  
#include "Engine/Core/Light/PointLight.hpp"
#include "Engine/Core/Light/DirectionalLight.hpp"
#include "Engine/Core/Light/SpotLight.hpp"
#include "Engine/Core/Primitive/Quad.hpp"
#include "Engine/Core/Primitive/Sphere.hpp"
#include "Engine/Core/Primitive/Point.hpp"
#include "Engine/Core/Primitive/Line.hpp"
#include "Engine/Core/Profiler/ProfileSystem.hpp"
#include "Engine/Math/MathUtils.hpp"

ProtoState::ProtoState()
{
	Renderer* theRenderer = Renderer::GetInstance();
	m_sceneGraph = new RenderSceneGraph();
	m_forwardPath = new ForwardRenderPath();
	m_forwardPath->m_renderer = theRenderer;

	Window* window = Window::GetInstance();
	float width = window->GetWindowWidth();
	float height = window->GetWindowHeight();
	float aspect = width / height;

	m_cameraInitialPos = Vector3(0.f, 0.f, -7.f);
	m_cameraRotationSpd = 50.f;

	if (!m_camera)
	{
		m_camera = new Camera(m_cameraInitialPos);
		m_camera->SetColorTarget(theRenderer->GetDefaultColorTarget());
		m_camera->SetDepthStencilTarget(theRenderer->GetDefaultDepthTarget());
		m_camera->SetAspect(aspect);
		m_camera->SetFOV(45.f);

		m_camera->SetPerspectiveProjection(m_camera->GetFOV(), aspect, 1.f, 1000.f);

		m_sceneGraph->AddCamera(m_camera);
	}

	if (!m_UICamera)
	{
		m_UICamera = new Camera();
		m_UICamera->SetColorTarget(theRenderer->GetDefaultColorTarget());
		m_UICamera->SetDepthStencilTarget(theRenderer->GetDefaultDepthTarget());
		m_UICamera->SetAspect(aspect);
		
		// ortho projection
		m_UICamera->SetProjectionOrtho(window->GetWindowWidth(), window->GetWindowHeight(), 0.f, 100.f);

		m_sceneGraph->m_camera2D = m_UICamera;
	}

	q_0 = new Quad(Vector3(-10.f, 0.f, 0.f), Vector3(0.f, 0.f, 0.f), Vector3(1.f, 1.f, 1.f), Rgba::MEGENTA, "quad_pcu", "default", false);
	m_gameObjects.push_back(q_0);

	Quad* q_1 = new Quad(Vector3(-10.f, 0.f, 5.f), Vector3::ZERO, Vector3(3.f, 0.2f, 1.f), Rgba::CYAN, "quad_pcu", "white", false);
	m_gameObjects.push_back(q_1);

	Quad* q_2 = new Quad(Vector3(-10.f, 0.5f, 5.f), Vector3::ZERO, Vector3(3.f, 0.2f, 1.f), Rgba::GREEN, "quad_pcu", "white", false, false, COMPARE_GREATER);
	m_gameObjects.push_back(q_2);
	
	Quad* q_3 = new Quad(Vector3(-10.f, -0.5f, 5.f), Vector3::ZERO, Vector3(3.f, 0.2f, 1.f), Rgba::BLUE, "quad_pcu", "white", false, false, COMPARE_ALWAYS);
	m_gameObjects.push_back(q_3);

	////////////////////////////////////////////////
	// quad surrounding point light, face up
	Quad* q_4 = new Quad(Vector3(0.f, 4.7f, 10.f), Vector3(-90.f, 0.f, 0.f), Vector3::ONE, Rgba::WHITE, "quad_lit", "lit", false, true);
	q_4->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_4);
	m_sceneGraph->AddRenderable(q_4);		// lit in forward path

	// face down
	Quad* q_5 = new Quad(Vector3(0.f, 6.3f, 10.f), Vector3(90.f, 0.f, 0.f), Vector3::ONE, Rgba::WHITE, "quad_lit", "lit", false, true);
	q_5->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_5);
	m_sceneGraph->AddRenderable(q_5);		// lit in forward path

	// face backward
	Quad* q_6 = new Quad(Vector3(0.f, 5.5f, 10.8f), Vector3(0.f, 0.f, 0.f), Vector3::ONE, Rgba::WHITE, "quad_lit", "lit", false, true);
	q_6->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_6);
	m_sceneGraph->AddRenderable(q_6);		// lit in forward path

	// face forward
	Quad* q_7 = new Quad(Vector3(0.f, 5.5f, 9.2f), Vector3(0.f, 180.f, 0.f), Vector3::ONE, Rgba::WHITE, "quad_lit", "lit", false, true);
	q_7->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_7);
	m_sceneGraph->AddRenderable(q_7);		// lit in forward path

	// face left
	Quad* q_8 = new Quad(Vector3(0.8f, 5.5f, 10.f), Vector3(0.f, -90.f, 0.f), Vector3::ONE, Rgba::WHITE, "quad_lit", "lit", false, true);
	q_8->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_8);
	m_sceneGraph->AddRenderable(q_8);		// lit in forward path

	// face right
	Quad* q_9 = new Quad(Vector3(-0.8f, 5.5f, 10.f), Vector3(0.f, 90.f, 0.f), Vector3::ONE, Rgba::WHITE, "quad_lit", "lit", false, true);
	q_9->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_9);
	m_sceneGraph->AddRenderable(q_9);		// lit in forward path
	////////////////////////////////////////////////////////////////////

	// spot light test quad
	Quad* q_10 = new Quad(Vector3(-10.f, 4.5f, 10.f), Vector3(0.f, 0.f, 0.f), Vector3(5.f, 5.f, 5.f), Rgba::WHITE, "quad_lit", "lit", false, true);
	q_10->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_10);
	m_sceneGraph->AddRenderable(q_10);

	// directional light test quad
	Quad* q_11 = new Quad(Vector3(-15.f, 4.5f, 10.f), Vector3(-90.f, 0.f, 0.f), Vector3(3.f, 3.f, 3.f), Rgba::WHITE, "quad_lit", "lit", false, true);
	q_11->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(q_11);
	m_sceneGraph->AddRenderable(q_11);

	c_0 = new Cube(Vector3(-5.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::GREEN, "cube_pcu", "default");
	m_gameObjects.push_back(c_0);
	m_sceneGraph->AddRenderable(c_0);

	c_1 = new Cube(Vector3::ZERO, Vector3::ZERO, Vector3::ONE, Rgba::RED, "cube_pcu", "default");
	m_gameObjects.push_back(c_1);

	s_0 = new Sphere(Vector3(10.f, 0.f, 0.f), Vector3::ZERO, Vector3::ONE, Rgba::WHITE, "sphere_pcu", "default");
	m_gameObjects.push_back(s_0);

	ship_0 = new Ship(Vector3(10.f, 0.f, 10.f), Vector3::ZERO, Vector3(0.3f, 0.3f, 0.3f), Rgba::WHITE, "ship_lit", "ship_multipass", true);
	ship_0->m_renderable->GetMaterial()->m_useLight = true;
	m_gameObjects.push_back(ship_0);
	m_sceneGraph->AddRenderable(ship_0);

	Ship* ship_1 = new Ship(Vector3(0.f, 0.f, 10.f), Vector3::ZERO, Vector3(0.5f, 0.5f, 0.5f), Rgba::WHITE, "ship_pcu", "ship");
	m_gameObjects.push_back(ship_1);

	// point
	p_0 = new Point(Vector3(5.f, 0.f, 0.f), Vector3::ZERO, 10.f, Rgba::WHITE, "point_pcu", "white");
	m_gameObjects.push_back(p_0);

	// line
	l_0 = new Line(Vector3(0.f, 5.f, 0.f), Vector3(5.f, 5.f, 0.f), 3.f, Rgba::WHITE, "white");
	m_gameObjects.push_back(l_0);

	// lights 
	Vector3 lightPos = Vector3(0.f, 5.5f, 10.f);
	Vector3 lightRot = Vector3::ZERO;
	Vector3 lightScale = Vector3(0.05f, 0.05f, 0.05f);
	Rgba lightColor = Rgba::WHITE;
	Vector3 attenuaion = Vector3(0.f, 0.f, .001f);
	Vector3 specAttenuation = Vector3(0.f, 0.f, .001f);
	Vector3 lightDir = Vector3::ONE;
	float innerAngle = -1.f;
	float outerAngle = -1.f;
	float directFac = 0.f;
	PointLight* pl_0 = new PointLight(lightPos, lightRot, lightScale, lightColor, attenuaion, specAttenuation, lightDir, innerAngle, outerAngle, directFac);
	m_gameObjects.push_back(pl_0);
	m_sceneGraph->AddLight(pl_0);

	lightPos = Vector3(10.f, 5.f, 10.f);
	lightColor = Rgba::RED;
	PointLight* pl_1 = new PointLight(lightPos, lightRot, lightScale, lightColor, attenuaion, specAttenuation, lightDir, innerAngle, outerAngle, directFac);
	m_gameObjects.push_back(pl_1);
	m_sceneGraph->AddLight(pl_1);

	lightPos = Vector3(-10.f, 4.5f, 8.f);
	lightColor = Rgba::GREEN;
	lightDir = Vector3(0.f, 0.f, 1.f);
	innerAngle = 1.f;
	outerAngle = CosDegrees(30.f);
	directFac = 0.f;
	SpotLight* sl_0 = new SpotLight(lightPos, lightRot, lightScale, lightColor, attenuaion, specAttenuation, lightDir, innerAngle, outerAngle, directFac);
	m_gameObjects.push_back(sl_0);
	m_sceneGraph->AddLight(sl_0);

	lightPos = Vector3(-15.f, 6.5f, 10.f);
	lightColor = Rgba::CYAN;
	attenuaion = Vector3(1.f, 0.f, 0.f);
	specAttenuation = Vector3(1.f, 0.f, 0.f);
	lightDir = Vector3(0.f, -1.f, 0.f);
	innerAngle = -1.f;
	outerAngle = -1.f;
	directFac = 1.f;
	DirectionalLight* dl_0 = new DirectionalLight(lightPos, lightRot, lightScale, lightColor, attenuaion, specAttenuation, lightDir, innerAngle, outerAngle, directFac);
	m_gameObjects.push_back(dl_0);
	m_sceneGraph->AddLight(dl_0);

	// debug
	DebugRenderSet3DCamera(m_camera);
	DebugRenderSet2DCamera(m_UICamera);

	m_control = CONTROL_CUBE;
}


ProtoState::~ProtoState()
{

}


void ProtoState::Update(float deltaTime)
{
	for each (GameObject* gameobject in m_gameObjects)
	{
		gameobject->Update(deltaTime);
	}

	if (!DevConsoleIsOpen())
	{
		UpdateMouse(deltaTime);
		UpdateKeyboard(deltaTime);
	}

	DebugRenderUpdate(deltaTime);
}


void ProtoState::UpdateMouse(float deltaTime)
{
	if (!IsProfilerOn())
	{
		// Apply Rotation
		Vector2 mouseDelta = g_input->GetMouseDelta();

		// local euler offset, starts at zero
		Vector3 localEuler = Vector3::ZERO; 
		localEuler.y = mouseDelta.x * m_cameraRotationSpd * deltaTime; 
		localEuler.x = mouseDelta.y * m_cameraRotationSpd * deltaTime; 

		float camEulerPitch = ClampFloat(m_camera->GetTransform().GetLocalRotation().x + localEuler.x, -90.f, 90.f);
		float camEulerYaw = fmod(m_camera->GetTransform().GetLocalRotation().y + localEuler.y, 360.f);
		float camEulerRoll = m_camera->GetTransform().GetLocalRotation().z;

		m_camera->GetTransform().SetLocalRotation(Vector3(camEulerPitch, camEulerYaw, camEulerRoll));
		Matrix44 model = m_camera->GetTransform().GetLocalMatrixEulerTranspose();

		Matrix44 inverseModel = model.Invert();
		m_camera->SetView(inverseModel);
		//m_camera->GetView().Print();
	}
}


void ProtoState::UpdateKeyboard(float deltaTime)
{
	// Apply movement
	float leftRight = 0.f;
	float forwardBack = 0.f;
	float upDown = 0.f; 
	
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_B))
	{
		if (m_control != (NUM_PRIMITIVE_CONTROL - 1))
		{
			m_control = static_cast<ePrimitiveControl>(m_control + 1);
		}
		else
		{
			m_control = CONTROL_CUBE;
		}
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_C))
	{
		for each (GameObject* gameobject in m_gameObjects)
		{
			gameobject->m_drawBasis = !gameobject->m_drawBasis;
		}
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_V))
	{
		for each (GameObject* gameobject in m_gameObjects)
		{
			gameobject->m_debugOn = !gameobject->m_debugOn;
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_A))
	{
		leftRight = -10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_D))
	{
		leftRight = 10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_W))
	{
		forwardBack = 10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_S))
	{
		forwardBack = -10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_Q))
	{
		upDown = 10.f;
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_E))
	{
		upDown = -10.f;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_Z))
	{
		// Reset camera position, euler and scale 
		m_camera->GetTransform().SetLocalPosition(m_cameraInitialPos);
		m_camera->GetTransform().SetLocalRotation(Vector3::ZERO);
		m_camera->GetTransform().SetLocalScale(Vector3::ONE);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_R))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.SetLocalRotation(Vector3::ZERO);
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.SetLocalRotation(Vector3::ZERO);
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_LEFT_ARROW))
	{
		// rotate around y
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.RotateLocal(Vector3(0.f, 1.f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.RotateLocal(Vector3(0.f, 1.f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_RIGHT_ARROW))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.RotateLocal(Vector3(0.f, -1.f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.RotateLocal(Vector3(0.f, -1.f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_UP_ARROW))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.RotateLocal(Vector3(1.f, 0.f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.RotateLocal(Vector3(1.f, 0.f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_DOWN_ARROW))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.RotateLocal(Vector3(-1.f, 0.f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.RotateLocal(Vector3(-1.f, 0.f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEUP))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.RotateLocal(Vector3(0.f, 0.f, 1.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.RotateLocal(Vector3(0.f, 0.f, 1.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_PAGEDOWN))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.RotateLocal(Vector3(0.f, 0.f, -1.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.RotateLocal(Vector3(0.f, 0.f, -1.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_I))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.TranslateLocal(Vector3(0.f, 0.f, .05f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.TranslateLocal(Vector3(0.f, 0.f, .05f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_K))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.TranslateLocal(Vector3(0.f, 0.f, -.05f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.TranslateLocal(Vector3(0.f, 0.f, -.05f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_J))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.TranslateLocal(Vector3(-0.05f, 0.f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.TranslateLocal(Vector3(-0.05f, 0.f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_L))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.TranslateLocal(Vector3(0.05f, 0.f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.TranslateLocal(Vector3(0.05f, 0.f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_U))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.TranslateLocal(Vector3(0.f, 0.05f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.TranslateLocal(Vector3(0.f, 0.05f, 0.f));
		}
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_O))
	{
		if (m_control == CONTROL_CUBE)
		{
			c_1->m_renderable->m_transform.TranslateLocal(Vector3(0.f, -0.05f, 0.f));
		}
		else if (m_control == CONTROL_SHPERE)
		{
			s_0->m_renderable->m_transform.TranslateLocal(Vector3(0.f, -0.05f, 0.f));
		}
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_F))
	{
		DebugRenderTasksFlush();
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_X) && DebugRenderOn())
	{
		Window* window = Window::GetInstance();
		//Renderer* renderer = Renderer::GetInstance();

		float windowWidth = window->GetWindowWidth();
		float windowHeight = window->GetWindowHeight();
		float lifetime = 1000.f;
		//float normalLifetime = 10000.f;
		//float quadWidth3D = 1.f;
		//float quadHeight3D = 1.f;
		//float pointSize = 5.f;
		Rgba startTimeColor = Rgba::RED;
		Rgba endTimeColor = Rgba::GREEN;
		eDebugDrawMode mode = DEBUG_RENDER_USE_DEPTH;
		AABB3 wireframeBound = AABB3(Vector3(-0.5f, 10.f, 0.f), Vector3(.5f, 11.f, 1.f));
		Vector3 drawmin = Vector3(-3.5f, 10.f, 0.f);
		Vector3 up = Vector3(0.f, 1.f, 0.f);
		Vector3 right = Vector3(-1.f, 0.f, 0.f);
		Vector3 pointPos = Vector3(5.f, 10.f, 0.f);
		Vector2 drawmin2D = Vector2(windowWidth / 4.f, 0.f);
		//Vector2 drawminChar2D = Vector2(-windowWidth / 4.f, 0.f);
		float textHeight = windowHeight / 40.f;
		Vector2 drawminText2D = Vector2(-windowWidth / 2.f, windowHeight / 2.f - textHeight);
		Vector3 lineStart = Vector3(10.f, 5.f, 0.f);
		Vector3 lineEnd = Vector3(15.f, 5.f, 0.f);
		float lineThickness = 1.f;
		//BitmapFont* font = renderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont.png");

		DebugRender2DText("This is from debug renderer", lifetime, drawminText2D, textHeight, .7f, startTimeColor, endTimeColor, mode);

		DebugRenderWireAABB3(lifetime, wireframeBound, startTimeColor, endTimeColor, mode);

		//DebugRenderQuad(lifetime, drawmin, up, right, quadWidth3D, quadHeight3D, startTimeColor, endTimeColor, mode);
		//DebugRender2DQuad(lifetime, drawmin2D, windowWidth/ 4.f, windowHeight / 4.f, startTimeColor, endTimeColor, mode, "2d_default");
		//DebugRender2DChar('H', lifetime, drawminChar2D, windowHeight / 4.f, 1.f, startTimeColor, endTimeColor, mode, font);

		// debug line not using line game object; instead it uses draw immediate of renderer
		DebugRenderLine(lifetime, lineStart, lineEnd, lineThickness, startTimeColor, endTimeColor, mode);

		DebugRenderBasis(lifetime, Vector3::ZERO, Vector3(1.f, 0.f, 0.f), 
			Vector3(0.f, 1.f, 0.f), Vector3(0.f, 0.f, 1.f), 
			lineThickness, mode);

		Vector3 gridBL = Vector3(-100.f, 0.f, -100.f);
		Vector3 gridBR = Vector3(100.f, 0.f,-100.f);
		Vector3 gridTL = Vector3(-100.f, 0.f, 100.f);
		Vector3 gridTR = Vector3(100.f, 0.f, 100.f);
		DebugRenderPlaneGrid(lifetime, gridBL, gridTL, gridTR, gridBR, 10.f, 10.f, 2.5f, mode);

		DebugRenderPoint(lifetime, 10.f, pointPos, startTimeColor, endTimeColor, mode);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_1))
	{
		// lift ambience
		Vector4 ambience = g_renderer->GetAmbience();
		float intensity = ClampFloat(ambience.w + 0.005f, 0.f, 1.f);
		float r = ClampFloat(ambience.x + 0.005f, 0.f, 1.f);
		float g = ClampFloat(ambience.y + 0.005f, 0.f, 1.f);
		float b = ClampFloat(ambience.z + 0.005f, 0.f, 1.f);
		Vector4 newAmbience = Vector4(r, g, b, intensity);
		g_renderer->SetAmbience(newAmbience);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_2))
	{
		// lower ambience
		Vector4 currentAmbience = g_renderer->GetAmbience();
		float intensity = ClampFloat(currentAmbience.w - 0.005f, 0.f, 1.f);
		float r = ClampFloat(currentAmbience.x - 0.005f, 0.f, 1.f);
		float g = ClampFloat(currentAmbience.y - 0.005f, 0.f, 1.f);
		float b = ClampFloat(currentAmbience.z - 0.005f, 0.f, 1.f);
		Vector4 newAmbience = Vector4(r, g, b, intensity);
		g_renderer->SetAmbience(newAmbience);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_4))
	{
		// Lifts spec amount 
		float specAmount = g_renderer->m_lightConstData.specularAmount;
		float newSpecAmount = ClampFloat(specAmount + 0.005f, 0.f, 1.f);
		g_renderer->m_lightConstData.specularAmount = newSpecAmount;
	
		ship_0->m_renderable->GetMaterial()->m_channel->SetProperty("SPECULAR_AMOUNT", newSpecAmount);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_5))
	{
		// Lowers spec amount 
		float specAmount = g_renderer->m_lightConstData.specularAmount;
		float newSpecAmount = ClampFloat(specAmount - 0.005f, 0.f, 1.f);
		g_renderer->m_lightConstData.specularAmount = newSpecAmount;
	
		ship_0->m_renderable->GetMaterial()->m_channel->SetProperty("SPECULAR_AMOUNT", newSpecAmount);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_7))
	{
		// Lifts spec power
		float specPower = g_renderer->m_lightConstData.specularPower;
		float newSpecPower = specPower + .1f;
		g_renderer->m_lightConstData.specularPower = newSpecPower;

		ship_0->m_renderable->GetMaterial()->m_channel->SetProperty("SPECULAR_POWER", newSpecPower);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_8))
	{
		// Lowers spec power
		float specPower = g_renderer->m_lightConstData.specularPower;
		float newSpecPower = specPower - .1f;
		if (newSpecPower <= 1.f)
		{
			// Clamp on one end
			newSpecPower = 1.f;
		}
		g_renderer->m_lightConstData.specularPower = newSpecPower;

		ship_0->m_renderable->GetMaterial()->m_channel->SetProperty("SPECULAR_POWER", newSpecPower);
	}
	if (g_input->IsKeyDown(InputSystem::KEYBOARD_NUMPAD_9))
	{
		// test normal that object should follow
		m_normalTestAngle += 0.1f;

		float x = CosDegrees(m_normalTestAngle);
		float y = SinDegrees(m_normalTestAngle);

		Vector3 start = Vector3::ZERO;
		Vector3 normalTest = Vector3(x, y, 0.f).GetNormalized() * 5.f;
		Vector3 end = start + normalTest;

		m_normalTestStart = start;
		m_normalTestEnd = end;

		// update euler of object to follow the normal
		Vector3 right = c_1->m_renderable->m_transform.GetLocalRight().GetNormalized();
		Vector3 up = (m_normalTestEnd - m_normalTestStart).GetNormalized();
		Vector3 forward = right.Cross(up);

		Matrix44 rotationalMat = Matrix44::FromBasis(right, up, forward);

		Vector3 euler = Matrix44::DecomposeMatrixIntoEuler(rotationalMat);

		c_1->m_renderable->m_transform.SetLocalRotation(euler);
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_0) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_NONE;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_1) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_COLOR;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_2) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_UV;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_3) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_SURFACE_NORMAL;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_4) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_WORLD_NORMAL;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_8) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_DIFFUSE;
	}
	if (g_input->WasKeyJustPressed(InputSystem::KEYBOARD_9) && DebugRenderOn())
	{
		g_renderer->m_debugModeData.mode.w = DEBUG_SPECULAR;
	}

	// camera update from input
	Vector3 camForward = m_camera->GetLocalForward(); 
	Vector3 camUp = m_camera->GetLocalUp(); 
	Vector3 camRight = m_camera->GetLocalRight(); 

	Vector3 worldOffset = camRight * leftRight * deltaTime
		+ camUp * upDown * deltaTime
		+ camForward * forwardBack * deltaTime;

	// Apply world offset
	m_camera->GetTransform().TranslateLocal(worldOffset); 
}


void ProtoState::Render(Renderer* renderer)
{
	renderer->SetCamera(m_camera);
	renderer->ClearScreen(Rgba::BLACK);

	for each (GameObject* gameobject in m_gameObjects)
	{
		if (!gameobject->m_isInForwardPath)
		{
			gameobject->Render(renderer);
		}

		gameobject->RenderBasis(renderer);
	}

	// normal following rotation test
	Shader* basisShader = renderer->CreateOrGetShader("direct");
	renderer->UseShader(basisShader);
	Texture* basisTexture = renderer->CreateOrGetTexture("Data/Images/white.png");
	renderer->SetTexture2D(0, basisTexture);
	renderer->SetSampler2D(0, basisTexture->GetSampler());
	renderer->m_objectData.model = Matrix44::IDENTITY;
	renderer->DrawLine3D(m_normalTestStart, m_normalTestEnd, Rgba::CYAN, 5.f);

	m_forwardPath->RenderScene(m_sceneGraph);
}