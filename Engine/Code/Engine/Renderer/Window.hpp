#pragma once

#include <vector>

typedef bool (*windows_message_handler_cb)( unsigned int msg, size_t wparam, size_t lparam );		// returns void, but takes specified arguments

class Window
{
public:
	// Do all the window creation work is currently in App
	Window(float aspect);
	~Window();

	// Register a function callback to the Window.  Any time Windows processing a 
	// message, this callback should forwarded the information, along with the
	// supplied user argument. 
	void RegisterHandler( windows_message_handler_cb cb ); 

	// Allow users to unregister (not needed for this Assignment, but I 
	// like having cleanup methods). 
	void UnregisterHandler( windows_message_handler_cb cb ); 
	void UnregisterCurrentHandlers();

	// This is safely castable to an HWND
	std::vector<windows_message_handler_cb>& GetListeners() {return m_listeners;}

	// ** EXTRAS ** //
	// void SetTitle( char const *new_title ); 

private:
	float m_width;
	float m_height;

	void* m_hwnd; // intptr_t  

	// When the WindowsProcedure is called - let all listeners get first crack at the message
	// Giving us better code modularity. 
	std::vector<windows_message_handler_cb> m_listeners; 

	static Window* m_windowInstance;
	static constexpr float ASPECT = 1.5f;

public:
	static Window*	GetInstance();
	static void		DestroyWindow();

	void* GetHandle() const	{ return m_hwnd; }
	float GetWindowWidth()	{return m_width;}
	float GetWindowHeight()	{return m_height;}
};