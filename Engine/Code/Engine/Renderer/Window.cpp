#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/GLFunctions.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Console/DevConsole.hpp"

typedef bool (*windows_message_handler_cb)( unsigned int msg, size_t wparam, size_t lparam ); 

// For singleton style classes, I like to had the instance variable within the CPP; 
Window* Window::m_windowInstance = new Window(ASPECT);
const char* APP_NAME = "Win32 OpenGL Test App";

// I believe in App you will have a windows procedure you use that looks similar to the following; 
// This will be moved to Windows.cpp (here), with a slight tweak; 
LRESULT CALLBACK GameWndProc( HWND hwnd, 
	UINT msg, 
	WPARAM wparam, 
	LPARAM lparam )
{
	bool callDefault = true;

	// NEW:  Give the custom handlers a chance to run first; 
	Window *window = Window::GetInstance(); 
	if (nullptr != window)
	{
		for (int i = 0; i < window->GetListeners().size(); ++i) {
			windows_message_handler_cb theFunc = window->GetListeners()[i];
			callDefault = (callDefault && !theFunc( msg, wparam, lparam )); 
		}
	}

	TODO("may implement msg handling that only window cares about in the future");
	// Do what you were doing before - some of this may have to be moved; 
	/*
	switch (msg) 
	{
		//...
	}; 
	*/

	// do default windows behaviour (return before this if you don't want default windows behaviour for certain messages)
	if (callDefault)
	{
		return ::DefWindowProc( hwnd, msg, wparam, lparam );
	}
	else
	{
		return 0;
	}
}


Window* Window::GetInstance()
{
	return m_windowInstance;
}


void Window::DestroyWindow()
{
	delete m_windowInstance;
	m_windowInstance = nullptr;
}


Window::Window(float aspect)
{
	// Define a window style/class
	WNDCLASSEX windowClassDescription;
	memset( &windowClassDescription, 0, sizeof( windowClassDescription ) );
	windowClassDescription.cbSize = sizeof( windowClassDescription );
	windowClassDescription.style = CS_OWNDC; // Redraw on move, request own Display Context
	windowClassDescription.lpfnWndProc = static_cast< WNDPROC >( GameWndProc ); // Register our Windows message-handling function
	windowClassDescription.hInstance = GetModuleHandle( NULL );
	windowClassDescription.hIcon = NULL;
	windowClassDescription.hCursor = NULL;
	windowClassDescription.lpszClassName = TEXT( "Simple Window Class" );
	RegisterClassEx( &windowClassDescription );

	const DWORD windowStyleFlags = WS_CAPTION | WS_BORDER | WS_THICKFRAME | WS_SYSMENU | WS_OVERLAPPED;
	const DWORD windowStyleExFlags = WS_EX_APPWINDOW;

	// Get desktop rect, dimensions, aspect
	RECT desktopRect;
	HWND desktopWindowHandle = GetDesktopWindow();
	GetClientRect( desktopWindowHandle, &desktopRect );
	float desktopWidth = (float)( desktopRect.right - desktopRect.left );
	float desktopHeight = (float)( desktopRect.bottom - desktopRect.top );
	float desktopAspect = desktopWidth / desktopHeight;

	// Calculate maximum client size (as some % of desktop size)
	constexpr float maxClientFractionOfDesktop = 0.90f;
	float clientWidth = desktopWidth * maxClientFractionOfDesktop;
	float clientHeight = desktopHeight * maxClientFractionOfDesktop;
	if( aspect > desktopAspect )
	{
		// Client window has a wider aspect than desktop; shrink client height to match its width
		clientHeight = clientWidth / aspect;
	}
	else
	{
		// Client window has a taller aspect than desktop; shrink client width to match its height
		clientWidth = clientHeight * aspect;
	}

	// Calculate client rect bounds by centering the client area
	float clientMarginX = 0.5f * (desktopWidth - clientWidth);
	float clientMarginY = 0.5f * (desktopHeight - clientHeight);
	RECT clientRect;
	clientRect.left = (int) clientMarginX;
	clientRect.right = clientRect.left + (int) clientWidth;
	clientRect.top = (int) clientMarginY;
	clientRect.bottom = clientRect.top + (int) clientHeight;

	m_width = clientWidth;
	m_height = clientHeight;

	// Calculate the outer dimensions of the physical window, including frame et. al.
	RECT windowRect = clientRect;
	AdjustWindowRectEx( &windowRect, windowStyleFlags, FALSE, windowStyleExFlags );

	WCHAR windowTitle[ 1024 ];
	MultiByteToWideChar( GetACP(), 0, APP_NAME, -1, windowTitle, sizeof( windowTitle ) / sizeof( windowTitle[ 0 ] ) );
	m_hwnd = CreateWindowEx(
		windowStyleExFlags,
		windowClassDescription.lpszClassName,
		windowTitle,
		windowStyleFlags,
		windowRect.left,
		windowRect.top,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		windowClassDescription.hInstance,
		NULL );

	if( !m_hwnd )
	{
		printf("Window not created %d\n", GetLastError() );
	}

	ShowWindow( (HWND)m_hwnd, SW_SHOW );
	SetForegroundWindow( (HWND)m_hwnd );
	SetFocus( (HWND)m_hwnd );

	HCURSOR cursor = LoadCursor( NULL, IDC_ARROW );
	SetCursor( cursor );

	RenderStartup((HWND)m_hwnd, m_width, m_height);

	DevConsole* console = DevConsole::GetInstance();
	Vector2 devConsoleBoxMin = Vector2(0.f, 0.f);
	Vector2 devConsoleBoxMax = Vector2(m_width, m_height);
	console->SetCamOrtho(AABB2(devConsoleBoxMin, devConsoleBoxMax));
	console->ConfigureSections();
}


Window::~Window()
{
	UnregisterCurrentHandlers();

	m_hwnd = nullptr;
}


void Window::RegisterHandler( windows_message_handler_cb cb )
{
	m_listeners.push_back(cb);
}


void Window::UnregisterHandler( windows_message_handler_cb cb)
{
	std::vector<windows_message_handler_cb>::iterator itr;
	itr = std::find(m_listeners.begin(), m_listeners.end(), cb);
	if (itr != m_listeners.end())
	{
		m_listeners.erase(itr);
	}
}


void Window::UnregisterCurrentHandlers()
{
	m_listeners.clear();
}