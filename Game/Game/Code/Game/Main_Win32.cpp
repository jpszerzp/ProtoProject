﻿#include "Game/TheApp.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Blackboard.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Util/StringUtils.hpp"
#include "Engine/Core/Time/TheTime.hpp"
#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Renderer/Window.hpp"

#include <math.h>
#include <cassert>
#include <crtdbg.h>
#include <ctime>
#include <random>

TheApp* g_theApp = nullptr;

bool HandleMsg(unsigned int msg, size_t wparam, size_t)
{
	unsigned char keyCode = (unsigned char) wparam;

	switch (msg)
	{
		case WM_CLOSE:
		{
			g_theApp->OnQuitRequested();
			return true;
		}

		case WM_KEYDOWN:
		{
			g_input->OnKeyPressed(keyCode);
			return true;
		}

		// Raw physical keyboard "key-was-just-released" event (case-insensitive, not translated)
		case WM_KEYUP:
		{
			g_input->OnKeyReleased(keyCode);
			return true;
		}

		case WM_LBUTTONDOWN:
		{
			g_input->SetMouseLeftButtonDown(true);
			return true;
		}

		case WM_LBUTTONUP:
		{
			g_input->SetMouseLeftButtonDown(false);
			return true;
		}

		case WM_RBUTTONDOWN:
		{
			g_input->SetMouseRightButtonDown(true);
			return true;
		}

		case WM_RBUTTONUP:
		{
			g_input->SetMouseRightButtonDown(false);
			return true;
		}

		case WM_MOUSEWHEEL:
		{
			short wheel = GET_WHEEL_DELTA_WPARAM(wparam); 
			float wheel_delta = (float)wheel / (float)WHEEL_DELTA; 

			// set m_frame_wheel_delta to 0 at the start of frame, before processing windows messages
			g_input->m_frameWheelDelta += wheel_delta; 

			return true;
		}

		case WM_ACTIVATE:
		{
			return true;
		}

		case WM_MOVE:
		{
			return true;
		}

		case WM_SIZE:
		{
			return true;
		}

		case WM_MOUSEMOVE:
		{
			return true;
		}

		default:
			return false;
	}
}

//-----------------------------------------------------------------------------------------------
void Initialize( HINSTANCE )
{
	g_theApp = new TheApp();

	Window* theWindow = Window::GetInstance();
	theWindow->RegisterHandler(HandleMsg);

	// app hook to engine common blackboard
	g_config_blackboard->app = g_theApp;
}

//-----------------------------------------------------------------------------------------------
void Shutdown()
{
	GLShutdown();

	Window::DestroyWindow();

	// Destroy the global App instance
	delete g_theApp;	
	g_theApp = nullptr;
}

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE applicationInstanceHandle, HINSTANCE, LPSTR commandLineString, int )
{
	UNUSED(commandLineString);
	srand((unsigned int)time( NULL ));

	Initialize(applicationInstanceHandle);

	// Program main loop; keep running frames until it's time to quit
	while(!g_theApp->IsQuitting())
	{
		g_theApp->RunFrame();
	}

	Shutdown();
	return 0;
}