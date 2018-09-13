#pragma once

#include "Engine/Input/KeyButtonState.hpp"
#include "Engine/Input/AnalogJoyStick.hpp"

enum XBoxButtons
{
	XBUTTON_GAMEPAD_DPAD_UP,					// 0
	XBUTTON_GAMEPAD_DPAD_DOWN,					// 1
	XBUTTON_GAMEPAD_DPAD_LEFT,					// 2
	XBUTTON_GAMEPAD_DPAD_RIGHT,					// 3
	XBUTTON_GAMEPAD_START,						// 4
	XBUTTON_GAMEPAD_BACK,						// 5
	XBUTTON_GAMEPAD_LEFT_THUMB,					// 6
	XBUTTON_GAMEPAD_RIGHT_THUMB,				// 7
	XBUTTON_GAMEPAD_LEFT_SHOULDER,				// 8
	XBUTTON_GAMEPAD_RIGHT_SHOULDER,				// 9
	XBUTTON_GAMEPAD_A,							// 10
	XBUTTON_GAMEPAD_B,							// 11
	XBUTTON_GAMEPAD_X,							// 12
	XBUTTON_GAMEPAD_Y,							// 13
	NUM_OF_BUTTON								// 14
};

class XboxController 
{
public:
	KeyButtonState m_buttonStates[XBoxButtons::NUM_OF_BUTTON];

	AnalogJoyStick leftJoyStick;
	AnalogJoyStick rightJoyStick;

	int m_controllerID;
	float m_normalizedLeftTriggerValue;
	float m_normalizedRightTriggerValue;

	XboxController();
	XboxController(int id);
	~XboxController();

	bool IsLeftJSTilted() const;
	bool IsRightJSTilted() const;
	bool IsConnected();

	float GetLeftJSTiltDirectionDegree() const;
	float GetLeftJSTiltDirectionRadian() const;
	float GetLeftJSTiltExtent() const;
	float GetRightJSTiltDirectionDegree() const;
	float GetRightJSTiltDirectionRadian() const;
	float GetRightJSTiltExtent() const;
	float GetNormalizedLeftTrigger() const { return m_normalizedLeftTriggerValue; }
	float GetNormalizedRightTrigger() const { return m_normalizedRightTriggerValue; }

	bool WasButtonJustPressed(unsigned char buttonCode);
	bool WasButtonJustReleased(unsigned char buttonCode);
	bool IsButtonDown(unsigned char buttonCode);
	void RestoreButtons();

	void OnButtonPressed(unsigned char buttonCode);
	void OnButtonReleased(unsigned char buttonCode);

	void UpdateController();
	void UpdateButtons();
	void UpdateLeftJoyStick();
	void UpdateRightJoyStick();
	void UpdateTriggers();
};