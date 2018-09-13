
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Window.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

InputSystem* InputSystem::m_inputInstance = nullptr;

const unsigned char InputSystem::KEYBOARD_UP_ARROW = VK_UP;
const unsigned char InputSystem::KEYBOARD_DOWN_ARROW = VK_DOWN;
const unsigned char InputSystem::KEYBOARD_LEFT_ARROW = VK_LEFT;
const unsigned char InputSystem::KEYBOARD_RIGHT_ARROW = VK_RIGHT;
const unsigned char InputSystem::KEYBOARD_A = 0x41;
const unsigned char InputSystem::KEYBOARD_B = 0x42;
const unsigned char InputSystem::KEYBOARD_C = 0x43;
const unsigned char InputSystem::KEYBOARD_D = 0x44;
const unsigned char InputSystem::KEYBOARD_E = 0x45;
const unsigned char InputSystem::KEYBOARD_F = 0x46;
const unsigned char InputSystem::KEYBOARD_G = 0x47;
const unsigned char InputSystem::KEYBOARD_H = 0x48;
const unsigned char InputSystem::KEYBOARD_I = 0x49;
const unsigned char InputSystem::KEYBOARD_J = 0x4A;
const unsigned char InputSystem::KEYBOARD_K = 0x4B;
const unsigned char InputSystem::KEYBOARD_L = 0x4C;
const unsigned char InputSystem::KEYBOARD_M = 0x4D;
const unsigned char InputSystem::KEYBOARD_N = 0x4E;
const unsigned char InputSystem::KEYBOARD_O = 0x4F;
const unsigned char InputSystem::KEYBOARD_P = 0x50;
const unsigned char InputSystem::KEYBOARD_Q = 0x51;
const unsigned char InputSystem::KEYBOARD_R = 0x52;
const unsigned char InputSystem::KEYBOARD_S = 0x53;
const unsigned char InputSystem::KEYBOARD_T = 0x54;
const unsigned char InputSystem::KEYBOARD_U = 0x55;
const unsigned char InputSystem::KEYBOARD_V = 0x56;
const unsigned char InputSystem::KEYBOARD_W = 0x57;
const unsigned char InputSystem::KEYBOARD_X = 0x58;
const unsigned char InputSystem::KEYBOARD_Y = 0x59;
const unsigned char InputSystem::KEYBOARD_Z = 0x5A;
const unsigned char InputSystem::KEYBOARD_F1 = VK_F1;
const unsigned char InputSystem::KEYBOARD_F2 = VK_F2;
const unsigned char InputSystem::KEYBOARD_F3 = VK_F3;
const unsigned char InputSystem::KEYBOARD_F4 = VK_F4;
const unsigned char InputSystem::KEYBOARD_F5 = VK_F5;
const unsigned char InputSystem::KEYBOARD_F6 = VK_F6;
const unsigned char InputSystem::KEYBOARD_F7 = VK_F7;
const unsigned char InputSystem::KEYBOARD_F8 = VK_F8;
const unsigned char InputSystem::KEYBOARD_F9 = VK_F9;
const unsigned char InputSystem::KEYBOARD_F10 = VK_F10;
const unsigned char InputSystem::KEYBOARD_F11 = VK_F11;
const unsigned char InputSystem::KEYBOARD_F12 = VK_F12;
const unsigned char InputSystem::KEYBOARD_0 = 0x30;
const unsigned char InputSystem::KEYBOARD_1 = 0x31;
const unsigned char InputSystem::KEYBOARD_2 = 0x32;
const unsigned char InputSystem::KEYBOARD_3 = 0x33;
const unsigned char InputSystem::KEYBOARD_4 = 0x34;
const unsigned char InputSystem::KEYBOARD_5 = 0x35;
const unsigned char InputSystem::KEYBOARD_6 = 0x36;
const unsigned char InputSystem::KEYBOARD_7 = 0x37;
const unsigned char InputSystem::KEYBOARD_8 = 0x38;
const unsigned char InputSystem::KEYBOARD_9 = 0x39;
const unsigned char InputSystem::KEYBOARD_NUMPAD_0 = 0x60;
const unsigned char InputSystem::KEYBOARD_NUMPAD_1 = 0x61;
const unsigned char InputSystem::KEYBOARD_NUMPAD_2 = 0x62;
const unsigned char InputSystem::KEYBOARD_NUMPAD_3 = 0x63;
const unsigned char InputSystem::KEYBOARD_NUMPAD_4 = 0x64;
const unsigned char InputSystem::KEYBOARD_NUMPAD_5 = 0x65;
const unsigned char InputSystem::KEYBOARD_NUMPAD_6 = 0x66;
const unsigned char InputSystem::KEYBOARD_NUMPAD_7 = 0x67;
const unsigned char InputSystem::KEYBOARD_NUMPAD_8 = 0x68;
const unsigned char InputSystem::KEYBOARD_NUMPAD_9 = 0x69;
const unsigned char InputSystem::KEYBOARD_SPACE	= VK_SPACE;
const unsigned char InputSystem::KEYBOARD_ESC = VK_ESCAPE;
const unsigned char InputSystem::KEYBOARD_ENTER = VK_RETURN;
const unsigned char InputSystem::KEYBOARD_SHIFT = VK_SHIFT;
const unsigned char InputSystem::KEYBOARD_CONTROL = VK_CONTROL;
const unsigned char InputSystem::KEYBOARD_ALT = VK_MENU;
const unsigned char InputSystem::KEYBOARD_CAPITAL = VK_CAPITAL;
const unsigned char InputSystem::KEYBOARD_PAGEUP = VK_PRIOR;
const unsigned char InputSystem::KEYBOARD_PAGEDOWN = VK_NEXT;
const unsigned char InputSystem::KEYBOARD_END = VK_END;
const unsigned char InputSystem::KEYBOARD_HOME = VK_HOME;
const unsigned char InputSystem::KEYBOARD_DELETE = VK_DELETE;
const unsigned char InputSystem::KEYBOARD_SEMICOLON_COLON = VK_OEM_1;
const unsigned char InputSystem::KEYBOARD_SLASH_QUESTIONMARK = VK_OEM_2;
const unsigned char InputSystem::KEYBOARD_DOT_WAVE = VK_OEM_3;
const unsigned char InputSystem::KEYBOARD_BACKSPACE = VK_BACK;
const unsigned char InputSystem::KEYBOARD_TAB = VK_TAB;
const unsigned char InputSystem::KEYBOARD_OEM_COMMA = VK_OEM_COMMA;
const unsigned char InputSystem::KEYBOARD_OEM_PERIOD = VK_OEM_PERIOD;
const unsigned char InputSystem::KEYBOARD_OEM_1 = VK_OEM_1;
const unsigned char InputSystem::KEYBOARD_OEM_2 = VK_OEM_2;
const unsigned char InputSystem::KEYBOARD_OEM_3 = VK_OEM_3;
const unsigned char InputSystem::KEYBOARD_OEM_4 = VK_OEM_4;
const unsigned char InputSystem::KEYBOARD_OEM_5 = VK_OEM_5;
const unsigned char InputSystem::KEYBOARD_OEM_6 = VK_OEM_6;
const unsigned char InputSystem::KEYBOARD_OEM_7 = VK_OEM_7;
const unsigned char InputSystem::KEYBOARD_OEM_8 = VK_OEM_8;

const unsigned char InputSystem::MOUSE_LBUTTON = VK_LBUTTON;
const unsigned char InputSystem::MOUSE_RBUTTON = VK_RBUTTON;
const unsigned char InputSystem::MOUSE_MBUTTON = VK_MBUTTON;


void RunMessagePump()
{
	MSG queuedMessage;
	for( ;; )
	{
		const BOOL wasMessagePresent = PeekMessage( &queuedMessage, NULL, 0, 0, PM_REMOVE );
		if( !wasMessagePresent )
		{
			break;
		}

		TranslateMessage( &queuedMessage );
		DispatchMessage( &queuedMessage ); // This tells Windows to call our "WindowsMessageHandlingProcedure" function
	}
}


InputSystem::InputSystem()
{
	for (int i = 0; i < NUM_KEYS; ++i)
	{
		m_keyStates[i] = KeyButtonState();
	}

	// MAY need to initialize xboxcontrollers when inputsystem is created
	for (int i = 0; i < NUM_CONTROLLERS; ++i)
	{
		m_controllers[i] = XboxController(i);
	}

	//m_mode = MOUSEMODE_ABSOLUTE;
	m_mode = MOUSEMODE_RELATIVE;
	m_positionThisFrame = Vector2::ZERO;
	m_positionLastFrame = Vector2::ZERO;
}


InputSystem::~InputSystem()
{

}


void InputSystem::BeginFrame()
{
	UpdateKeyboard();
	UpdateControllers();
	RunMessagePump();
}


void InputSystem::Update()
{
	// Absolute Mode - I get mouse position - and I can potentially lock to the screen
	m_positionLastFrame = m_positionThisFrame; 
	m_positionThisFrame = GetMouseClientPosition();

	if (m_mouseLock)
	{
		m_mode = MOUSEMODE_RELATIVE;
	}
	else 
	{
		m_mode = MOUSEMODE_ABSOLUTE;
	}

	// Relative mode -> I care about deltas - I reset to the center (meaning, mutually exclusive modes)
	if (m_mode == MOUSEMODE_RELATIVE) {
		m_positionLastFrame = GetCenterOfClientWindow(); 
		SetMouseClientPosition( m_positionLastFrame ); 
	}
}


void InputSystem::EndFrame()
{

}


void InputSystem::OnKeyPressed(unsigned char keyCode)
{
	if (!m_keyStates[keyCode].m_isDown)
	{
		m_keyStates[keyCode].m_isDown = true;
		m_keyStates[keyCode].m_wasJustPressed = true;
	}
}


void InputSystem::OnKeyReleased(unsigned char keyCode)
{
	m_keyStates[keyCode].m_isDown = false;
	m_keyStates[keyCode].m_wasJustReleased = true;
}


bool InputSystem::IsKeyDown( unsigned char keyCode ) const
{
	return m_keyStates[keyCode].m_isDown; 
}


bool InputSystem::WasKeyJustPressed( unsigned char keyCode ) const
{
	return m_keyStates[keyCode].m_wasJustPressed;
}


bool InputSystem::WasKeyJustReleased( unsigned char keyCode ) const
{
	return m_keyStates[keyCode].m_wasJustReleased;
}


void InputSystem::UpdateControllers()
{
	// poll and update controller status
	for (int i = 0; i < NUM_CONTROLLERS; ++i)
	{
		m_controllers[i].UpdateController();
	}
}


InputSystem* InputSystem::GetInstance()
{
	if (m_inputInstance == nullptr)
	{
		m_inputInstance = new InputSystem();
	}

	return m_inputInstance;
}

void InputSystem::DestroyInstance()
{
	delete m_inputInstance;
	m_inputInstance = nullptr;
}

void InputSystem::UpdateKeyboard()
{
	// Clear all just-changed flags, in preparation for the next round of WM_KEYDOWN, etc. messages
	for( int keyCode = 0; keyCode < InputSystem::NUM_KEYS; ++keyCode )
	{
		m_keyStates[ keyCode ].m_wasJustPressed = false;
		m_keyStates[ keyCode ].m_wasJustReleased = false;
	}
}


Vector2 InputSystem::GetMouseClientPosition() 
{
	POINT desktopPos;
	::GetCursorPos( &desktopPos ); 

	HWND hwnd = (HWND)Window::GetInstance()->GetHandle(); 

	::ScreenToClient( hwnd, &desktopPos ); 
	POINT clientPos = desktopPos; 

	return Vector2( (float)clientPos.x, (float)clientPos.y ); 
}


Vector2 InputSystem::GetCenterOfClientWindow()
{
	float xPos = 0.f;
	float yPos = 0.f;

	RECT rect;
	HWND hwnd = (HWND)Window::GetInstance()->GetHandle();

	bool succeed = ::GetClientRect(hwnd, &rect);
	if (succeed)
	{
		xPos = (rect.left + rect.right) / 2.f;
		yPos = (rect.top + rect.bottom) / 2.f;
	}

	return Vector2(xPos, yPos);
}


void InputSystem::SetMouseScreenPosition(Vector2 desktopPos)
{
	::SetCursorPos( (int)desktopPos.x, (int)desktopPos.y ); 
}


void InputSystem::SetMouseClientPosition(Vector2 clientPos)
{
	HWND hwnd = (HWND)Window::GetInstance()->GetHandle();

	POINT cPos;
	cPos.x = (LONG)clientPos.x;
	cPos.y = (LONG)clientPos.y;

	::ClientToScreen(hwnd, &cPos);

	SetMouseScreenPosition(Vector2((float)cPos.x, (float)cPos.y));
}


Vector2 InputSystem::GetMouseDelta()
{
	return m_positionThisFrame - m_positionLastFrame;
}


void InputSystem::MouseShowCursor(bool show)
{
	::ShowCursor(show);
}


void InputSystem::MouseLockCursor(bool lock)
{
	if (!lock) {
		::ClipCursor( nullptr ); // this unlock the mouse
	} else {
		HWND hwnd = (HWND)Window::GetInstance()->GetHandle();

		RECT client_rect; // window class RECT
		::GetClientRect( hwnd, &client_rect ); 

		POINT offset; 
		offset.x = 0; 
		offset.y = 0; 
		::ClientToScreen( hwnd, &offset ); 

		client_rect.left += offset.x; 
		client_rect.right += offset.x; 
		client_rect.top += offset.y; 
		client_rect.bottom += offset.y; 

		::ClipCursor( &client_rect ); 
	}
}