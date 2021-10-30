#include "Window.h"
#include <iostream>

double Window::s_MouseWheelY = 0.0;

Window::Window(uint32_t width, uint32_t height, const char* title, bool bFullscreen)
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
		__debugbreak();
	}

	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Setting callbacks
	glfwSetScrollCallback(m_Window, ScrollCallback);
}

Window::~Window()
{
	Destroy();
}

void Window::Update()
{
	//Resetting the mouse wheel state
	s_MouseWheelY = 0.0;
	glfwPollEvents();
	
	if (IsKeyboardEvent({GLFW_KEY_ESCAPE, GLFW_PRESS}))
		glfwSetWindowShouldClose(m_Window, true);

	glfwSwapBuffers(m_Window);
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

void Window::SetWndInCurrentContext()
{
	glfwMakeContextCurrent(m_Window);
}

bool Window::IsKeyboardEvent(const InputEvent& ie) const
{
	return (glfwGetKey(m_Window, ie.Key()) == ie.State());
}

bool Window::IsMouseEvent(const InputEvent& ie) const
{
	return (glfwGetMouseButton(m_Window, ie.Key()) == ie.State());
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
