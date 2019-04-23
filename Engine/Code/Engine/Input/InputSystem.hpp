#pragma once

#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/KeyButtonState.hpp"

enum eMouseMode
{
	MOUSEMODE_RELATIVE,
	MOUSEMODE_ABSOLUTE
};

class InputSystem
{
public:
	static const int NUM_KEYS = 256;
	static const int NUM_CONTROLLERS = 4;

	static const unsigned char KEYBOARD_UP_ARROW;
	static const unsigned char KEYBOARD_DOWN_ARROW;
	static const unsigned char KEYBOARD_LEFT_ARROW;
	static const unsigned char KEYBOARD_RIGHT_ARROW;
	static const unsigned char KEYBOARD_A;
	static const unsigned char KEYBOARD_B;
	static const unsigned char KEYBOARD_C;
	static const unsigned char KEYBOARD_D;
	static const unsigned char KEYBOARD_E;
	static const unsigned char KEYBOARD_F;
	static const unsigned char KEYBOARD_G;
	static const unsigned char KEYBOARD_H;
	static const unsigned char KEYBOARD_I;
	static const unsigned char KEYBOARD_J;
	static const unsigned char KEYBOARD_K;
	static const unsigned char KEYBOARD_L;
	static const unsigned char KEYBOARD_M;
	static const unsigned char KEYBOARD_N;
	static const unsigned char KEYBOARD_O;
	static const unsigned char KEYBOARD_P;
	static const unsigned char KEYBOARD_Q;
	static const unsigned char KEYBOARD_R;
	static const unsigned char KEYBOARD_S;
	static const unsigned char KEYBOARD_T;
	static const unsigned char KEYBOARD_U;
	static const unsigned char KEYBOARD_V;
	static const unsigned char KEYBOARD_W;
	static const unsigned char KEYBOARD_X;
	static const unsigned char KEYBOARD_Y;
	static const unsigned char KEYBOARD_Z;
	static const unsigned char KEYBOARD_F1;
	static const unsigned char KEYBOARD_F2;
	static const unsigned char KEYBOARD_F3;
	static const unsigned char KEYBOARD_F4;
	static const unsigned char KEYBOARD_F5;
	static const unsigned char KEYBOARD_F6;
	static const unsigned char KEYBOARD_F7;
	static const unsigned char KEYBOARD_F8;
	static const unsigned char KEYBOARD_F9;
	static const unsigned char KEYBOARD_F10;
	static const unsigned char KEYBOARD_F11;
	static const unsigned char KEYBOARD_F12;
	static const unsigned char KEYBOARD_0;
	static const unsigned char KEYBOARD_1;
	static const unsigned char KEYBOARD_2;
	static const unsigned char KEYBOARD_3;
	static const unsigned char KEYBOARD_4;
	static const unsigned char KEYBOARD_5;
	static const unsigned char KEYBOARD_6;
	static const unsigned char KEYBOARD_7;
	static const unsigned char KEYBOARD_8;
	static const unsigned char KEYBOARD_9;
	static const unsigned char KEYBOARD_NUMPAD_0;
	static const unsigned char KEYBOARD_NUMPAD_1;
	static const unsigned char KEYBOARD_NUMPAD_2;
	static const unsigned char KEYBOARD_NUMPAD_3;
	static const unsigned char KEYBOARD_NUMPAD_4;
	static const unsigned char KEYBOARD_NUMPAD_5;
	static const unsigned char KEYBOARD_NUMPAD_6;
	static const unsigned char KEYBOARD_NUMPAD_7;
	static const unsigned char KEYBOARD_NUMPAD_8;
	static const unsigned char KEYBOARD_NUMPAD_9;
	static const unsigned char KEYBOARD_SPACE;
	static const unsigned char KEYBOARD_ESC;
	static const unsigned char KEYBOARD_ENTER;
	static const unsigned char KEYBOARD_SHIFT;
	static const unsigned char KEYBOARD_CONTROL;
	static const unsigned char KEYBOARD_ALT;
	static const unsigned char KEYBOARD_CAPITAL;
	static const unsigned char KEYBOARD_PAGEUP;
	static const unsigned char KEYBOARD_PAGEDOWN;
	static const unsigned char KEYBOARD_END;
	static const unsigned char KEYBOARD_HOME;
	static const unsigned char KEYBOARD_DELETE;
	static const unsigned char KEYBOARD_SEMICOLON_COLON;
	static const unsigned char KEYBOARD_SLASH_QUESTIONMARK;
	static const unsigned char KEYBOARD_DOT_WAVE;
	static const unsigned char KEYBOARD_BACKSPACE;
	static const unsigned char KEYBOARD_TAB;
	static const unsigned char KEYBOARD_OEM_COMMA;
	static const unsigned char KEYBOARD_OEM_PERIOD;
	static const unsigned char KEYBOARD_OEM_1;
	static const unsigned char KEYBOARD_OEM_2;
	static const unsigned char KEYBOARD_OEM_3;
	static const unsigned char KEYBOARD_OEM_4;
	static const unsigned char KEYBOARD_OEM_5;
	static const unsigned char KEYBOARD_OEM_6;
	static const unsigned char KEYBOARD_OEM_7;
	static const unsigned char KEYBOARD_OEM_8;
	static const unsigned char KEYBOARD_OEM_PLUS;
	static const unsigned char KEYBOARD_OEM_MINUS;

	static const unsigned char MOUSE_LBUTTON;
	static const unsigned char MOUSE_RBUTTON;
	static const unsigned char MOUSE_MBUTTON;

	InputSystem();
	~InputSystem();

	void BeginFrame();
	void EndFrame();
	void Update();

	void OnKeyPressed( unsigned char keyCode );
	void OnKeyReleased( unsigned char keyCode );
	bool IsKeyDown( unsigned char keyCode ) const;
	bool WasKeyJustPressed( unsigned char keyCode ) const;
	bool WasKeyJustReleased( unsigned char keyCode ) const;

	XboxController&	GetController( int controllerID ) { return m_controllers[ controllerID ]; }

	bool GetSuppression() {return m_keyBoardSuppressController;}
	void SetSuppression(bool value) {m_keyBoardSuppressController = value;}

	// mouse
	void MouseLockCursor(bool lock);
	void MouseShowCursor(bool show);
	Vector2 GetMouseClientPosition();
	Vector2 GetCenterOfClientWindow();
	void SetMouseScreenPosition(Vector2 desktopPos);
	void SetMouseClientPosition(Vector2 clientPos);
	Vector2 GetMouseDelta();

	void SetMouseLeftButtonDown(bool l_down) { m_mouseLeftButtonDown = l_down; }
	void SetMouseRightButtonDown(bool r_down) { m_mouseRightButtonDown = r_down; }

	void UpdateKeyboard();
	void UpdateControllers();

	static InputSystem* GetInstance();
	static void DestroyInstance();

public:
	static InputSystem* m_inputInstance;

	KeyButtonState m_keyStates[NUM_KEYS];
	XboxController	m_controllers[ NUM_CONTROLLERS ];

	bool m_keyBoardSuppressController = false;
	bool m_mouseLeftButtonDown = false;
	bool m_mouseRightButtonDown = false;
	bool m_mouseLock = true;

	float m_frameWheelDelta;
	Vector2 m_positionLastFrame;
	Vector2 m_positionThisFrame;
	eMouseMode m_mode;
};