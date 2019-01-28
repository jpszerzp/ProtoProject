//#include "Game/CameraAnchor.hpp"
//#include "Game/GameCommon.hpp"
//#include "Engine/Renderer/Renderable.hpp"
//#include "Engine/Input/InputSystem.hpp"
//#include "Engine/Math/MathUtils.hpp"
//
//CameraAnchor::CameraAnchor()
//{
//
//}
//
//
//CameraAnchor::CameraAnchor(Vector3 pos, Vector3 rot, Vector3 scale)
//{
//	m_transform = Transform(pos, rot, scale);
//}
//
//
//CameraAnchor::~CameraAnchor()
//{
//
//}
//
//
//void CameraAnchor::Update(float deltaTime)
//{
//	UpdateInput(deltaTime);
//}
//
//
//void CameraAnchor::UpdateInput(float deltaTime)
//{
//	InputSystem* input = InputSystem::GetInstance();
//
//	if (input->m_mouseLock)
//	{
//		Vector2 mouseDelta = g_input->GetMouseDelta();
//
//		Vector3 localEuler = Vector3::ZERO; 
//		localEuler.y = mouseDelta.x * 50.f * deltaTime; 
//		localEuler.x = mouseDelta.y * 50.f * deltaTime; 
//
//		float eulerPitch = ClampFloat(m_transform.GetLocalRotation().x + localEuler.x, -90.f, 90.f);
//		float eulerYaw = fmod(m_transform.GetLocalRotation().y + localEuler.y, 360.f);
//		float eulerRoll = m_transform.GetLocalRotation().z;
//
//		m_transform.SetLocalRotation(Vector3(eulerPitch, eulerYaw, eulerRoll));
//	}
//}