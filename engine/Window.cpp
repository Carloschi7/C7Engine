#include "Window.h"
#include <iostream>
#include <utility>

void InputState::Update(const Window* window)
{
	m_OldState = m_CurrentState;

	//Printable keys
	for (u16 i = 32; i <= 162; i++)
		m_CurrentState.printable_keys[i - 32] = window->IsKeyboardEvent({ i, GLFW_PRESS });
	
	//Function keys
	for (u16 i = 256; i <= 348; i++)
		m_CurrentState.function_keys[i - 256] = window->IsKeyboardEvent({ i, GLFW_PRESS });

	//Mouse keys
	for (u16 i = 0; i <= 7; i++)
		m_CurrentState.mouse_keys[i] = window->IsMouseEvent({ i, GLFW_PRESS });
}

bool InputState::IsKeyPressed(u16 key) const
{
	switch (_GetKeyType(key))
	{
	case KeyType::Printable:
		return m_CurrentState.printable_keys[key - 32] && !m_OldState.printable_keys[key - 32];
	case KeyType::Function:
		return m_CurrentState.function_keys[key - 256] && !m_OldState.function_keys[key - 256];
	case KeyType::Mouse:
		return m_CurrentState.mouse_keys[key] && !m_OldState.mouse_keys[key];
	}

	return false;
}

bool InputState::IsKeyHeld(u16 key) const
{
	switch (_GetKeyType(key))
	{
	case KeyType::Printable:
		return m_CurrentState.printable_keys[key - 32] && m_OldState.printable_keys[key - 32];
	case KeyType::Function:
		return m_CurrentState.function_keys[key - 256] && m_OldState.function_keys[key - 256];
	case KeyType::Mouse:
		return m_CurrentState.mouse_keys[key] && m_OldState.mouse_keys[key];
	}

	return false;
}

bool InputState::IsKeyReleased(u16 key) const
{
	switch (_GetKeyType(key))
	{
	case KeyType::Printable:
		return !m_CurrentState.printable_keys[key - 32] && m_OldState.printable_keys[key - 32];
	case KeyType::Function:
		return !m_CurrentState.function_keys[key - 256] && m_OldState.function_keys[key - 256];
	case KeyType::Mouse:
		return !m_CurrentState.mouse_keys[key] && m_OldState.mouse_keys[key];
	}

	return false;
}

KeyType InputState::_GetKeyType(u16 key) const
{
	if (key < 8)
		return KeyType::Mouse;
	if (key >= 32 && key <= 162)
		return KeyType::Printable;
	if (key >= 256 && key <= 348)
		return KeyType::Function;

	return KeyType::None;
}

double Window::s_MouseWheelY = 0.0;

Window::Window(u32 width, u32 height, const char* title, bool bFullscreen)
	:m_Width(width), m_Height(height), m_Fullscreen(bFullscreen), m_Monitor(nullptr)
{
	if (m_Fullscreen)
	{
		m_Window = glfwCreateWindow(m_Width, m_Height, title, m_Monitor, nullptr);
		glfwSetWindowMonitor(m_Window, m_Monitor, 0, 0, m_Width, m_Height, 60);
	}
	else
	{
		m_Window = glfwCreateWindow(m_Width, m_Height, title, m_Monitor, nullptr);
	}

	if (!m_Window)
	{
		std::cout << "Could not open the window\n";
		throw std::runtime_error("Error in file Window.cpp");
	}

	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Setting callbacks
	glfwSetScrollCallback(m_Window, ScrollCallback);
}

Window::~Window()
{
	DetachWndFromContext();
	Destroy();
}

Window::Window(Window&& wnd) noexcept :
	m_Window(std::exchange(wnd.m_Window, nullptr)),
	m_Monitor(std::exchange(wnd.m_Monitor, nullptr)),
	m_Width(wnd.m_Width), m_Height(wnd.m_Height), 
	m_Fullscreen(wnd.m_Fullscreen)
{
}

void Window::Update() const
{
	//Resetting the mouse wheel state
	s_MouseWheelY = 0.0;
	glfwPollEvents();
	
	if (IsKeyboardEvent({GLFW_KEY_ESCAPE, GLFW_PRESS}))
		glfwSetWindowShouldClose(m_Window, true);

	glfwSwapBuffers(m_Window);
}

void Window::UpdateKeys()
{
	m_InputState.Update(this);
}

void Window::Destroy()
{
	if (m_Window)
	{
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}
	
	if (m_Monitor) delete m_Monitor;
}

void Window::ClearScreen(u32 flags)
{
	glClear(flags);
}

void Window::AttachWndToCurrentContext() const
{
	glfwMakeContextCurrent(m_Window);
}

void Window::DetachWndFromContext() const
{
	glfwMakeContextCurrent(nullptr);
}

bool Window::IsKeyboardEvent(const InputEvent& ie) const
{
	return (glfwGetKey(m_Window, ie.Key()) == ie.State());
}

bool Window::IsMouseEvent(const InputEvent& ie) const
{
	return (glfwGetMouseButton(m_Window, ie.Key()) == ie.State());
}

bool Window::IsKeyPressed(u16 key) const
{
	return m_InputState.IsKeyPressed(key);
}

bool Window::IsKeyHeld(u16 key) const
{
	return m_InputState.IsKeyHeld(key);
}

bool Window::IsKeyReleased(u16 key) const
{
	return m_InputState.IsKeyReleased(key);
}

bool Window::IsMouseWheelUp() const
{
	return s_MouseWheelY > 0.0;
}

bool Window::IsMouseWheelDown() const
{
	return s_MouseWheelY < 0.0;
}

void Window::GetCursorCoord(double& x, double& y) const
{
	glfwGetCursorPos(m_Window, &x, &y);
}

void Window::SetVsync(bool true_or_false) const
{
	glfwSwapInterval(true_or_false);
}

bool Window::ShouldClose() const
{
	return glfwWindowShouldClose(m_Window); 
}

void Window::EnableCursor()
{
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::DisableCursor()
{
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}




