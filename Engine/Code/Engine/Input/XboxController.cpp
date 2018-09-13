
#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"

#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include <Xinput.h>
#pragma comment( lib, "xinput9_1_0" ) // Link in the xinput.lib static library // #Eiserloh: Xinput 1_4 doesn't work in Windows 7; use 9_1_0 explicitly for broadest compatibility
#include <stdio.h>


XboxController::XboxController()
{

}


XboxController::~XboxController()
{

}


XboxController::XboxController(int id)
{
	m_controllerID = id;
}


bool XboxController::IsLeftJSTilted() const
{
	return (leftJoyStick.m_correctedRadius > 0.f);
}

bool XboxController::IsRightJSTilted() const
{
	return (rightJoyStick.m_correctedRadius > 0.f);
}


bool XboxController::IsConnected()
{
	// see if the controller is connected
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );	

	if (errorStatus == ERROR_SUCCESS)
	{
		return true;
	}
	else if (errorStatus == ERROR_NOT_CONNECTED)
	{
		printf("XBoxController %d is not connected", m_controllerID);
	}
	else
	{
		printf("XboxController %d reports error code %u (0x%08x)\n", m_controllerID, errorStatus, errorStatus);
	}

	return false;
}


float XboxController::GetLeftJSTiltDirectionDegree() const
{
	// given that joystick is tilted, give current tilt direction
	return leftJoyStick.m_angleDegree;
}


float XboxController::GetLeftJSTiltDirectionRadian() const
{
	return ConvertDegreesToRadians(leftJoyStick.m_angleDegree);
}


float XboxController::GetLeftJSTiltExtent() const
{
	return leftJoyStick.m_correctedRadius;
}


float XboxController::GetRightJSTiltDirectionDegree() const
{
	return rightJoyStick.m_angleDegree;
}


float XboxController::GetRightJSTiltDirectionRadian() const
{
	return ConvertDegreesToRadians(rightJoyStick.m_angleDegree);
}


float XboxController::GetRightJSTiltExtent() const
{
	return rightJoyStick.m_correctedRadius;
}


void XboxController::UpdateButtons()
{
	// refresh xbox buttons for upcoming frame
	RestoreButtons();

	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );

	if(errorStatus == ERROR_SUCCESS)
	{
		// A button pressed
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_A) == XINPUT_GAMEPAD_A)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_A);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_A);
		}

		// Start button pressed
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_START) == XINPUT_GAMEPAD_START)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_START);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_START);
		}

		// up button
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) == XINPUT_GAMEPAD_DPAD_UP)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_DPAD_UP);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_DPAD_UP);
		}

		// down
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) == XINPUT_GAMEPAD_DPAD_DOWN)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_DPAD_DOWN);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_DPAD_DOWN);
		}

		// left
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) == XINPUT_GAMEPAD_DPAD_LEFT)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_DPAD_LEFT);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_DPAD_LEFT);
		}

		// right
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) == XINPUT_GAMEPAD_DPAD_RIGHT)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_DPAD_RIGHT);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_DPAD_RIGHT);
		}

		// back
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) == XINPUT_GAMEPAD_BACK)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_BACK);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_BACK);
		}

		// left thumb
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) == XINPUT_GAMEPAD_LEFT_THUMB)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_LEFT_THUMB);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_LEFT_THUMB);
		}

		// right thumb
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) == XINPUT_GAMEPAD_RIGHT_THUMB)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_RIGHT_THUMB);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_RIGHT_THUMB);
		}

		// left shoulder
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) == XINPUT_GAMEPAD_LEFT_SHOULDER)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_LEFT_SHOULDER);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_LEFT_SHOULDER);
		}

		// right shoulder
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) == XINPUT_GAMEPAD_RIGHT_SHOULDER)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_RIGHT_SHOULDER);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_RIGHT_SHOULDER);
		}

		// B
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_B) == XINPUT_GAMEPAD_B)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_B);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_B);
		}

		// X
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_X) == XINPUT_GAMEPAD_X)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_X);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_X);
		}

		// Y
		if ((xboxControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_Y) == XINPUT_GAMEPAD_Y)
		{
			OnButtonPressed(XBoxButtons::XBUTTON_GAMEPAD_Y);
		}
		else 
		{
			OnButtonReleased(XBoxButtons::XBUTTON_GAMEPAD_Y);
		}
	}	
	else if (errorStatus == ERROR_NOT_CONNECTED)
	{
		printf("XBoxController %d is not connected", m_controllerID);
	}
	else
	{
		printf("XboxController %d reports error code %u (0x%08x)\n", m_controllerID, errorStatus, errorStatus);
	}
}


void XboxController::UpdateLeftJoyStick()
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );	

	if (errorStatus == ERROR_SUCCESS)
	{
		// get raw cartesians
		int rawLeftJoyStickX = xboxControllerState.Gamepad.sThumbLX;
		int rawLeftJoyStickY = xboxControllerState.Gamepad.sThumbLY;

		// map raw cartesian ([-32768, 32768]) to [-1, 1] for both x and y
		float correctedLeftJoyStickX = RangeMapFloat(static_cast<float>(rawLeftJoyStickX), -32768.f, 32768.f, -1.f, 1.f);
		float correctedLeftJoyStickY = RangeMapFloat(static_cast<float>(rawLeftJoyStickY), -32768.f, 32768.f, -1.f, 1.f);

		// convert to polar coords ([0, 1] for radius and [0, 360) for degree)
		float radiusLeftJoyStick = sqrtf( ( correctedLeftJoyStickX * correctedLeftJoyStickX ) + ( correctedLeftJoyStickY * correctedLeftJoyStickY ) );
		float degreeLeftJoyStick = ConvertRadiansToDegrees( static_cast<float>( atan2(correctedLeftJoyStickY, correctedLeftJoyStickX) ) );

		// remap the live zone to [0, 1]
		float innerRadiusLeftJoyStick = leftJoyStick.m_innerDeadZoneFraction * 1.f;
		float outerRadiusLeftJoyStick = leftJoyStick.m_outerDeadZoneFraction * 1.f;
		float correctedRadiusLeftJoyStick = RangeMapFloat(radiusLeftJoyStick, innerRadiusLeftJoyStick, outerRadiusLeftJoyStick, 0.f, 1.f);
		correctedRadiusLeftJoyStick = ClampZeroToOne(correctedRadiusLeftJoyStick);

		// update joy stick
		leftJoyStick.m_correctedRadius = correctedRadiusLeftJoyStick;
		leftJoyStick.m_angleDegree = degreeLeftJoyStick;
		leftJoyStick.m_rawCartesianPosition = Vector2(leftJoyStick.m_correctedRadius * CosDegrees(leftJoyStick.m_angleDegree), leftJoyStick.m_correctedRadius * SinDegrees(leftJoyStick.m_angleDegree));
	}
	else if (errorStatus == ERROR_NOT_CONNECTED)
	{
		printf("XBoxController %d is not connected", m_controllerID);
	}
	else
	{
		printf("XboxController %d reports error code %u (0x%08x)\n", m_controllerID, errorStatus, errorStatus);
	}
}


void XboxController::UpdateRightJoyStick()
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );	

	if (errorStatus == ERROR_SUCCESS)
	{
		// get raw cartesians
		int rawRightJoyStickX = xboxControllerState.Gamepad.sThumbRX;
		int rawRightJoyStickY = xboxControllerState.Gamepad.sThumbRY;

		// map raw cartesian ([-32768, 32768]) to [-1, 1] for both x and y
		float correctedRightJoyStickX = RangeMapFloat(static_cast<float>(rawRightJoyStickX), -32768.f, 32768.f, -1.f, 1.f);
		float correctedRightJoyStickY = RangeMapFloat(static_cast<float>(rawRightJoyStickY), -32768.f, 32768.f, -1.f, 1.f);

		// convert to polar coords ([0, 1] for radius and [0, 360) for degree)
		float radiusRightJoyStick = sqrtf( ( correctedRightJoyStickX * correctedRightJoyStickX ) + ( correctedRightJoyStickY * correctedRightJoyStickY ) );
		float degreeRightJoyStick = ConvertRadiansToDegrees( static_cast<float>( atan2(correctedRightJoyStickY, correctedRightJoyStickX) ) );

		// remap the live zone to [0, 1]
		float innerRadiusRightJoyStick = rightJoyStick.m_innerDeadZoneFraction * 1.f;
		float outerRadiusRightJoyStick = rightJoyStick.m_outerDeadZoneFraction * 1.f;
		float correctedRadiusRightJoyStick = RangeMapFloat(radiusRightJoyStick, innerRadiusRightJoyStick, outerRadiusRightJoyStick, 0.f, 1.f);
		correctedRadiusRightJoyStick = ClampZeroToOne(correctedRadiusRightJoyStick);

		// update joy stick
		rightJoyStick.m_correctedRadius = correctedRadiusRightJoyStick;
		rightJoyStick.m_angleDegree = degreeRightJoyStick;
		rightJoyStick.m_rawCartesianPosition = Vector2(rightJoyStick.m_correctedRadius * CosDegrees(rightJoyStick.m_angleDegree), rightJoyStick.m_correctedRadius * SinDegrees(rightJoyStick.m_angleDegree));
	}
	else if (errorStatus == ERROR_NOT_CONNECTED)
	{
		printf("XBoxController %d is not connected", m_controllerID);
	}
	else
	{
		printf("XboxController %d reports error code %u (0x%08x)\n", m_controllerID, errorStatus, errorStatus);
	}
}


void XboxController::UpdateTriggers()
{
	XINPUT_STATE xboxControllerState;
	memset( &xboxControllerState, 0, sizeof( xboxControllerState ) );
	DWORD errorStatus = XInputGetState( m_controllerID, &xboxControllerState );	

	if (errorStatus == ERROR_SUCCESS)
	{
		float rawLeftTrigger = xboxControllerState.Gamepad.bLeftTrigger;
		float rawRightTrigger = xboxControllerState.Gamepad.bRightTrigger;

		m_normalizedLeftTriggerValue = RangeMapFloat(rawLeftTrigger, 0.f, 255.f, 0.f, 1.f);
		m_normalizedRightTriggerValue = RangeMapFloat(rawRightTrigger, 0.f, 255.f, 0.f, 1.f);
	}
	else if (errorStatus == ERROR_NOT_CONNECTED)
	{
		printf("XBoxController %d is not connected", m_controllerID);
	}
	else
	{
		printf("XboxController %d reports error code %u (0x%08x)\n", m_controllerID, errorStatus, errorStatus);
	}
}


void XboxController::UpdateController()
{
	UpdateButtons();
	UpdateLeftJoyStick();
	UpdateRightJoyStick();
	UpdateTriggers();
}


bool XboxController::WasButtonJustPressed(unsigned char buttonCode)
{
	return (m_buttonStates[buttonCode].m_wasJustPressed);
}


bool XboxController::WasButtonJustReleased(unsigned char buttonCode)
{
	return (m_buttonStates[buttonCode].m_wasJustReleased);
}


void XboxController::OnButtonPressed(unsigned char buttonCode)
{
	KeyButtonState& button = m_buttonStates[buttonCode];

	// on button first pressed down
	if (!button.m_isDown)
	{
		button.m_isDown = true;
		button.m_wasJustPressed = true;
	}
}


void XboxController::OnButtonReleased(unsigned char buttonCode)
{
	KeyButtonState& button = m_buttonStates[buttonCode];

	// on button released
	if (button.m_isDown)
	{
		button.m_isDown = false;
		button.m_wasJustReleased = true;
	}
}


bool XboxController::IsButtonDown(unsigned char buttonCode)
{
	return (m_buttonStates[buttonCode].m_isDown);
}


void XboxController::RestoreButtons()
{
	for (int i = 0; i < XBoxButtons::NUM_OF_BUTTON; ++i)
	{
		m_buttonStates[i].m_wasJustPressed = false;
		m_buttonStates[i].m_wasJustReleased = false;
	}
}